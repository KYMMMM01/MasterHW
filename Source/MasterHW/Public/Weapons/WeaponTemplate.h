#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "WeaponTemplate.generated.h"

//Template Method 패턴 본체. Fire가 알고리즘 골격을 정의하고 각 단계는 자식이 override.
UCLASS(Abstract, Blueprintable)
class MASTERHW_API AWeaponTemplate : public AWeaponBase
{
	GENERATED_BODY()
public:
	virtual void Fire(AController* EventInstigator, FVector StartPos, FVector Direction) override;

	//재장전
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void Reload();
	virtual void Reload_Implementation();

protected:
	//탄약 확인 - 기본은 CurrentAmmo >= AmmoPerFire
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	bool CheckAmmo();
	virtual bool CheckAmmo_Implementation();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void PlayEffects();

	//실제 발사 (Shotgun이 여기를 override 해서 산탄으로 바꾼다)
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void ProcessFiring(AController* EventInstigator, FVector StartPos, FVector Direction);
	virtual void ProcessFiring_Implementation(AController* EventInstigator, FVector StartPos, FVector Direction);

	//탄약 차감
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void UpdateAmmo();
	virtual void UpdateAmmo_Implementation();
};
