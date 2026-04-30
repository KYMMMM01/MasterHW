#include "Weapons/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	DamageTypeClass = UDamageType::StaticClass();
}

bool AWeaponBase::CanFire() const
{
	UWorld* World = GetWorld();
	if (!World || FireRate <= 0.f) return true;
	return (World->GetTimeSeconds() - LastFireTime) >= (1.f / FireRate);
}

void AWeaponBase::Fire(AController* EventInstigator, FVector StartPos, FVector Direction)
{
	if (!CanFire()) return;
	LastFireTime = GetWorld()->GetTimeSeconds();

	TArray<AActor*> ActorsToIgnore;
	if (EventInstigator && EventInstigator->GetPawn())
		ActorsToIgnore.Add(EventInstigator->GetPawn());

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this,
		StartPos,
		StartPos + Direction.GetSafeNormal() * Range,
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
			Direction,
			HitResult,
			EventInstigator,
			this,
			DamageTypeClass
		);
	}
}
