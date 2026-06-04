// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class MASTERHW_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//OnHealthDamaged(현재체력, 최대체력, 변화량)에 바인딩 => UFUNCTION 필수
	UFUNCTION()
	void UpdateHealth(float NewHealth, float MaxHealth, float HealthChange);

protected:
	//WBP에서 같은 이름(HPBar)의 ProgressBar와 자동 연결
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;
};
