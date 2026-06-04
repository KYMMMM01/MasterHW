// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Weapons/WeaponBase.h"
#include "MasterHWCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHealthComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMasterHWCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//에디터에서 IA_Fire InputAction 에셋 연결
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

public:
	AMasterHWCharacter();

	//BeginPlay에서 이 클래스의 무기를 스폰한다 (블루프린트에서 BP_Shotgun 등으로 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	//체력/사망 델리게이트를 처리하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> HealthComp;

	//사망 후 리스폰까지 대기 시간(초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RespawnDelay = 3.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();

	//테스트용 콘솔 명령: 플레이 중 콘솔(~)을 열고 "DebugDamage" 입력 (예: DebugDamage 100)
	UFUNCTION(Exec)
	void DebugDamage(float Amount = 25.f);

	//OnHealthDead에 바인딩 => UFUNCTION 필수
	UFUNCTION()
	void HandleDeath(AController* DeathInstigator);

	//리스폰 처리(타이머 콜백)
	void Respawn();

	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	AWeaponBase* CurrentWeapon = nullptr;

	//아직 회복되지 않은 반동 누적값
	FVector2D RecoilAccum = FVector2D::ZeroVector;

	//사망 시점의 컨트롤러(리스폰 대상)
	UPROPERTY()
	TObjectPtr<AController> DeadController = nullptr;

	FTimerHandle RespawnTimerHandle;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
