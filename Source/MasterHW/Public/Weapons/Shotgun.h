#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Shotgun.generated.h"

UCLASS(Blueprintable)
class MASTERHW_API AShotgun : public AWeaponBase
{
	GENERATED_BODY()
public:
	AShotgun();

	//샷건 발사 로직
	virtual void Fire(AController* EventInstigator, FVector StartPos, FVector Direction) override;

	//한 번 발사 시 나가는 총알 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun")
	int32 NumBullets = 8;

	//확산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun")
	float Spread = 10.f;
};
