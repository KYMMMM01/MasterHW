#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponTemplate.h"
#include "Shotgun.generated.h"

//샷건 - Template Method의 ProcessFiring 한 단계만 override
UCLASS(Blueprintable)
class MASTERHW_API AShotgun : public AWeaponTemplate
{
	GENERATED_BODY()
public:
	AShotgun();

protected:
	//산탄 발사 => 단일 트레이스를 펠릿으로 교체
	virtual void ProcessFiring_Implementation(AController* EventInstigator, FVector StartPos, FVector Direction) override;

public:
	//한 번 발사 시 나가는 펠릿 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun")
	int32 NumBullets = 8;

	//확산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun")
	float Spread = 10.f;
};
