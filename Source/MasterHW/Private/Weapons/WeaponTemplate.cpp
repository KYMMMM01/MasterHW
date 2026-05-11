#include "Weapons/WeaponTemplate.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void AWeaponTemplate::Fire(AController* EventInstigator, FVector StartPos, FVector Direction)
{
	//쿨다운 확인
	if (!bCanFire) return;

	//탄약 확인 - 없으면 재장전하고 종료
	if (!CheckAmmo())
	{
		Reload();
		return;
	}

	//이펙트(블루프린트 구현)
	PlayEffects();

	//실제 발사
	ProcessFiring(EventInstigator, StartPos, Direction);

	//탄약 차감
	UpdateAmmo();

	//부모 호출 => 쿨다운 타이머 시작
	Super::Fire(EventInstigator, StartPos, Direction);
}

bool AWeaponTemplate::CheckAmmo_Implementation()
{
	return CurrentAmmo >= AmmoPerFire;
}

void AWeaponTemplate::UpdateAmmo_Implementation()
{
	CurrentAmmo -= AmmoPerFire;
}

void AWeaponTemplate::Reload_Implementation()
{
	CurrentAmmo = MaxAmmo;
	HandleFireDelay();
}

//기본 발사
void AWeaponTemplate::ProcessFiring_Implementation(AController* EventInstigator, FVector StartPos, FVector Direction)
{
	TArray<AActor*> ActorsToIgnore;
	if (EventInstigator && EventInstigator->GetPawn())
		ActorsToIgnore.Add(EventInstigator->GetPawn());

	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
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
