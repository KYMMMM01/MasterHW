// Fill out your copyright notice in the Description page of Project Settings.

#include "DELEGATE/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTake);
		UE_LOG(LogTemp, Warning, TEXT("[Health] BeginPlay bound on %s (HP %.0f)"), *Owner->GetName(), CurrentHealth);
	}
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//현재 HP를 화면에 계속 표시 (UI 붙이면 제거 가능)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green,
			FString::Printf(TEXT("HP : %.0f / %.0f"), CurrentHealth, MaxHealth));
	}
}

void UHealthComponent::DamageTake(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* Instigator, AActor* Causer)
{
	if (bIsDead || Damage <= 0.f)
	{
		return;
	}

	//0 이하로 안 내려가게
	const float FinalDamage = FMath::Min(Damage, CurrentHealth);
	CurrentHealth -= FinalDamage;

	//UI 갱신
	OnHealthDamaged.Broadcast(CurrentHealth, MaxHealth, FinalDamage);

	//[임시 검증용]
	UE_LOG(LogTemp, Warning, TEXT("[Health] -%.1f => %.1f / %.1f"), FinalDamage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnHealthDead.Broadcast(Instigator);

		UE_LOG(LogTemp, Warning, TEXT("[Health] Dead!"));
	}
}
