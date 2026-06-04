// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HealthBarWidget.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::UpdateHealth(float NewHealth, float MaxHealth, float HealthChange)
{
	if (HPBar && MaxHealth > 0.f)
	{
		HPBar->SetPercent(NewHealth / MaxHealth);
	}
}
