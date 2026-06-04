// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 데미지를 입었을 때 (현재체력, 최대체력, 변화량)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHealthDamagedSignature, float, NewHealth, float, MaxHealth, float, HealthChange);
//사망했을 때 (가해자 컨트롤러)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthDeadSignature, AController*, Instigator);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERHW_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	//UI 갱신용
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FHealthDamagedSignature OnHealthDamaged;

	//사망 처리용
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FHealthDeadSignature OnHealthDead;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 0.f;

	//중복 처리 방지
	bool bIsDead = false;

private:
	//OnTakeAnyDamage에 바인딩 => UFUNCTION 필수
	UFUNCTION()
	void DamageTake(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		AController* Instigator, AActor* Causer);
};
