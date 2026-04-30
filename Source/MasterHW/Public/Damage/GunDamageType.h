#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "GunDamageType.generated.h"

UCLASS(Blueprintable)
class MASTERHW_API UGunDamageType : public UDamageType
{
	GENERATED_BODY()
public:
	UGunDamageType();

	//샷건 데미지인지 구분
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	bool bIsShotgunDamage = false;

	//방어력 관통 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	float ArmorPenetration = 0.0f;
};
