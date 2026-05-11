#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "WeaponBase.generated.h"

class UArrowComponent;

//모든 무기의 데이터/쿨다운 베이스
UCLASS(Abstract, Blueprintable)
class MASTERHW_API AWeaponBase : public AActor
{
	GENERATED_BODY()
public:
	AWeaponBase();
	virtual void BeginPlay() override;

	//발사 => 부모는 쿨다운만
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire(AController* EventInstigator, FVector StartPos, FVector Direction);

	//컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> FirePoint;

	//데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Stats")
	float Damage = 20.f;

	//사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Stats")
	float Range = 5000.f;

	//초당 발사 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRate = 2.f;

	//한 번 발사 시 소모 탄약
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 AmmoPerFire = 1;

	//최대 탄약
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo = 12;

	//현재 탄약
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0;

	//수직 반동
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilPitch = 1.5f;

	//수평 반동
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilYawRange = 0.5f;

	//반동 회복 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilRecoverySpeed = 6.f;

	//데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	//발사 가능 여부
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bCanFire = true;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	FTimerHandle TimerFireDelay;

	//쿨다운이 끝나면 bCanFire를 다시 켠다
	UFUNCTION()
	void HandleFireDelay();
};
