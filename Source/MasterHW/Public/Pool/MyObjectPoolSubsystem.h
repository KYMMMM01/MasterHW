#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Pool/PooledObjectData.h"

#include "MyObjectPoolSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPoolerCleanupSignature);

USTRUCT(BlueprintType)
struct FSingleObjectPool
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TArray<TObjectPtr<class UPooledObject>> PooledObjects;
};

//레벨에 배치하던 매니저 액터(AMyObjectPool)를 월드와 수명을 같이하는 서브시스템으로 바꾼 버전
UCLASS(BlueprintType)
class MASTERHW_API UMyObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	//게임 월드(PIE 포함)에서만 생성되도록 제한
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UPROPERTY()
	FPoolerCleanupSignature OnPoolerCleanup;

	//배치 인스턴스가 없으므로 레시피는 레벨 블루프린트에서 주입받는다
	UFUNCTION(BlueprintCallable)
	void InitializePool(const TArray<FPooledObjectData>& Recipes);

	UFUNCTION(BlueprintCallable)
	void Broadcast_PoolerCleanup();

	UFUNCTION(BlueprintCallable)
	AActor* GetPooledActor(FString Name);

	UFUNCTION(BlueprintCallable)
	void RecyclePooledObject(class UPooledObject* PoolCompRef);

	UFUNCTION(BlueprintCallable)
	void RecycleActor(AActor* PooledActor);

protected:

	//레시피
	UPROPERTY(BlueprintReadOnly)
	TArray<FPooledObjectData> PooledObjectData;

	//만들어 둔 액터에 붙어있는 컴포넌트를 모아둔 장소
	UPROPERTY(BlueprintReadOnly)
	TArray<FSingleObjectPool> Pools;

private:
	void RegenItem(int32 PoolIndex, int32 PositionIndex);

	bool bInitialized = false;

};
