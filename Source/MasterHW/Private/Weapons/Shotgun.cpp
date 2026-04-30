#include "Weapons/Shotgun.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

AShotgun::AShotgun()
{
	Damage = 15.f; //발당 데미지
	FireRate = 0.8f; //연사 속도
	RecoilPitch = 5.f; //수직 반동
	RecoilYawRange = 1.5f; //수평 반동
}

void AShotgun::Fire(AController* EventInstigator, FVector StartPos, FVector Direction)
{
	if (!CanFire()) return;
	LastFireTime = GetWorld()->GetTimeSeconds();

	TArray<AActor*> ActorsToIgnore;
	if (EventInstigator && EventInstigator->GetPawn())
		ActorsToIgnore.Add(EventInstigator->GetPawn());

	for (int32 i = 0; i < NumBullets; ++i)
	{
		//Spread범위 안에서 랜덤 방향 생성
		FVector BulletDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(Direction, Spread);

		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::LineTraceSingle(
			this,
			StartPos,
			StartPos + BulletDir * Range,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			EDrawDebugTrace::ForDuration,
			HitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			2.f
		);

		if (bHit && HitResult.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(
				HitResult.GetActor(),
				Damage,
				BulletDir,
				HitResult,
				EventInstigator,
				this,
				DamageTypeClass
			);
		}
	}
}
