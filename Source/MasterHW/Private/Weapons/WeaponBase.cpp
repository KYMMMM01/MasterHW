#include "Weapons/WeaponBase.h"
#include "Components/ArrowComponent.h"
#include "TimerManager.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	//총구 위치/방향. 블루프린트 자식에서 위치 조절 가능
	FirePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(RootComponent);

	DamageTypeClass = UDamageType::StaticClass();
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MaxAmmo;
}

void AWeaponBase::Fire(AController* /*EventInstigator*/, FVector /*StartPos*/, FVector /*Direction*/)
{
	//부모는 쿨다운만 시작 - 실제 발사 흐름은 자식(WeaponTemplate)이 정의
	bCanFire = false;
	const float Delay = 1.f / FMath::Max(FireRate, 0.01f);
	GetWorld()->GetTimerManager().SetTimer(
		TimerFireDelay, this, &AWeaponBase::HandleFireDelay, Delay, false);
}

void AWeaponBase::HandleFireDelay()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerFireDelay);
	bCanFire = true;
}
