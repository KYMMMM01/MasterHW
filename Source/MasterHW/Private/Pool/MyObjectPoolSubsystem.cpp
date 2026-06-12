#include "Pool/MyObjectPoolSubsystem.h"
#include "Pool/PooledObject.h"

bool UMyObjectPoolSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

//액터 버전의 BeginPlay에서 하던 사전 스폰. 레벨 블루프린트 BeginPlay에서 호출해준다
void UMyObjectPoolSubsystem::InitializePool(const TArray<FPooledObjectData>& Recipes)
{
	//중복 호출되면 풀이 통째로 한 벌 더 생기므로 최초 1회만 허용
	if (bInitialized) { return; }
	bInitialized = true;

	PooledObjectData = Recipes;

	FActorSpawnParameters SpawnParams;
	for (int32 PoolIndex = 0; PoolIndex < PooledObjectData.Num(); PoolIndex++)
	{
		FSingleObjectPool CurrentPool;

		//레시피에 클래스가 비어있어도 Pools와 PooledObjectData의 인덱스 짝은 유지해야 함
		if (PooledObjectData[PoolIndex].ActorTemplate)
		{
			SpawnParams.Name = FName(*PooledObjectData[PoolIndex].ActorName);
			SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			for (int32 ObjectIndex = 0; ObjectIndex < PooledObjectData[PoolIndex].PoolSize; ObjectIndex++)
			{
				AActor* SpawnedActor = GetWorld()->SpawnActor(PooledObjectData[PoolIndex].ActorTemplate, &FVector::ZeroVector, &FRotator::ZeroRotator, SpawnParams);
#if WITH_EDITOR
				SpawnedActor->SetActorLabel(SpawnedActor->GetName());
#endif
				UPooledObject* PoolComp = NewObject<UPooledObject>(SpawnedActor);
				PoolComp->RegisterComponent();
				SpawnedActor->AddInstanceComponent(PoolComp);
				PoolComp->Init(this);
				CurrentPool.PooledObjects.Add(PoolComp);
				SpawnedActor->SetActorHiddenInGame(true);
				SpawnedActor->SetActorEnableCollision(false);
				SpawnedActor->SetActorTickEnabled(false);
			}
		}
		Pools.Add(CurrentPool);
	}
}

void UMyObjectPoolSubsystem::Broadcast_PoolerCleanup()
{
	OnPoolerCleanup.Broadcast();
}

AActor* UMyObjectPoolSubsystem::GetPooledActor(FString Name)
{
	int32 PoolCount = Pools.Num();
	int32 CurrentPoolIndex = -1;

	//요청한 이름이 존재하는지 확인
	for (int32 i = 0; i < PoolCount; i++)
	{
		if (PooledObjectData[i].ActorName == Name)
		{
			CurrentPoolIndex = i;
			break;
		}
	}

	//존재하지않다면 나가기
	if (CurrentPoolIndex == -1) { return nullptr; }

	int32 PooledObjectCount = Pools[CurrentPoolIndex].PooledObjects.Num();
	int32 FirstAvailable = -1;

	//우리가 찾은 풀 안에서 Active중이지 않은 첫번째 애를 찾기
	for (int32 i = 0; i < PooledObjectCount; i++)
	{
		if (Pools[CurrentPoolIndex].PooledObjects[i] != nullptr)
		{
			if (!Pools[CurrentPoolIndex].PooledObjects[i]->bIsPoolActive)
			{
				FirstAvailable = i;
				break;
			}
		}
		else //슬롯이 비어있다면 그 자리에 다시 만들기
		{
			RegenItem(CurrentPoolIndex, i);
			FirstAvailable = i;
			break;
		}
	}

	if (FirstAvailable >= 0)
	{
		//만들어진 오브젝트의 컴포넌트를 끌고와서 사용중으로 만들어줌
		UPooledObject* ToReturn = Pools[CurrentPoolIndex].PooledObjects[FirstAvailable];
		ToReturn->bIsPoolActive = true;

		//회수 예약
		OnPoolerCleanup.AddUniqueDynamic(ToReturn, &UPooledObject::RecycleSelf);

		//숨겨놨던걸 다시 보이게 해줌
		AActor* ToReturnActor = ToReturn->GetOwner();
		ToReturnActor->SetActorHiddenInGame(false);
		ToReturnActor->SetActorEnableCollision(true);
		ToReturnActor->SetActorTickEnabled(true);

		return ToReturnActor;
	}

	//전부 사용중이면 CanGrow가 켜진 풀만 새로 만들어서 줌
	if (!PooledObjectData[CurrentPoolIndex].bCanGrow || !PooledObjectData[CurrentPoolIndex].ActorTemplate) { return nullptr; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*PooledObjectData[CurrentPoolIndex].ActorName);
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor(PooledObjectData[CurrentPoolIndex].ActorTemplate, &FVector::ZeroVector, &FRotator::ZeroRotator, SpawnParams);
#if WITH_EDITOR
	SpawnedActor->SetActorLabel(SpawnedActor->GetName());
#endif
	UPooledObject* PoolComp = NewObject<UPooledObject>(SpawnedActor);
	PoolComp->RegisterComponent();
	SpawnedActor->AddInstanceComponent(PoolComp);
	PoolComp->Init(this);
	Pools[CurrentPoolIndex].PooledObjects.Add(PoolComp);
	PoolComp->bIsPoolActive = true;
	OnPoolerCleanup.AddUniqueDynamic(PoolComp, &UPooledObject::RecycleSelf);
	return SpawnedActor;
}

//최종적으로 정리해주는 로직
void UMyObjectPoolSubsystem::RecyclePooledObject(UPooledObject* PoolCompRef)
{
	if (!PoolCompRef) { return; }

	OnPoolerCleanup.RemoveDynamic(PoolCompRef, &UPooledObject::RecycleSelf);

	PoolCompRef->bIsPoolActive = false;
	AActor* ReturningActor = PoolCompRef->GetOwner();
	ReturningActor->SetActorHiddenInGame(true);
	ReturningActor->SetActorEnableCollision(false);
	ReturningActor->SetActorTickEnabled(false);
}

//블루프린트나 외부에서 호출해주면 반납해줌
void UMyObjectPoolSubsystem::RecycleActor(AActor* PooledActor)
{
	if (!PooledActor) { return; }

	if (UPooledObject* PoolCompRef = Cast<UPooledObject>(PooledActor->GetComponentByClass(UPooledObject::StaticClass())))
	{
		RecyclePooledObject(PoolCompRef);
	}
}

//파괴되어 비어버린 슬롯을 같은 자리에 다시 채워주기
void UMyObjectPoolSubsystem::RegenItem(int32 PoolIndex, int32 PositionIndex)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*PooledObjectData[PoolIndex].ActorName);
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor(PooledObjectData[PoolIndex].ActorTemplate, &FVector::ZeroVector, &FRotator::ZeroRotator, SpawnParams);
#if WITH_EDITOR
	SpawnedActor->SetActorLabel(SpawnedActor->GetName());
#endif
	UPooledObject* PoolComp = NewObject<UPooledObject>(SpawnedActor);
	PoolComp->RegisterComponent();
	SpawnedActor->AddInstanceComponent(PoolComp);
	PoolComp->Init(this);

	//원본은 Insert라서 죽은 null 슬롯이 배열에 그대로 남았음. 같은 자리에 대입해서 구멍을 메운다
	Pools[PoolIndex].PooledObjects[PositionIndex] = PoolComp;
	SpawnedActor->SetActorHiddenInGame(true);
	SpawnedActor->SetActorEnableCollision(false);
	SpawnedActor->SetActorTickEnabled(false);
}
