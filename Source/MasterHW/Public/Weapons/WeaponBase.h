#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "WeaponBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MASTERHW_API AWeaponBase : public AActor
{
	GENERATED_BODY()
public:
	AWeaponBase();

	//발사 (StartPos: 카메라 위치, Direction: 카메라 정면)
	virtual void Fire(AController* EventInstigator, FVector StartPos, FVector Direction);

	//연사 속도 확인
	bool CanFire() const;

	//데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float Damage = 20.f;
	
	//사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float Range = 5000.f;

	//연사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float FireRate = 2.f;

	
	//수직 반동 (위로 차오르는 정도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	float RecoilPitch = 1.5f;

	//수평 반동
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	float RecoilYawRange = 0.5f;

	//반동 회복 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	float RecoilRecoverySpeed = 6.f;

	//데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

protected:
	float LastFireTime = -999.f;
};
