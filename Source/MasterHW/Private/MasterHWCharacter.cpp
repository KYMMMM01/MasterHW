#include "MasterHWCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/DamageEvents.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMasterHWCharacter::AMasterHWCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AMasterHWCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	if (DefaultWeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);
		if (CurrentWeapon)
		{
			//스켈레탈 메시의 hand_r 소켓에 부착
			CurrentWeapon->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("hand_r")
			);
		}
	}
}

void AMasterHWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//반동 회복 => 누적된 반동을 매 프레임 조금씩 되돌린다
	if (!RecoilAccum.IsNearlyZero(0.01f) && CurrentWeapon && Controller)
	{
		float Speed = CurrentWeapon->RecoilRecoverySpeed;

		float RecoveryPitch = RecoilAccum.X * Speed * DeltaTime;
		float RecoveryYaw = RecoilAccum.Y * Speed * DeltaTime;

		//위로 올라간 반동 양수 Pitch 입력으로 아래로 내림
		AddControllerPitchInput(RecoveryPitch);
		//Yaw 반대 방향으로
		AddControllerYawInput(-RecoveryYaw);

		RecoilAccum.X -= RecoveryPitch;
		RecoilAccum.Y -= RecoveryYaw;
		
		if (FMath::Abs(RecoilAccum.X) < 0.01f) RecoilAccum.X = 0.f;
		if (FMath::Abs(RecoilAccum.Y) < 0.01f) RecoilAccum.Y = 0.f;
	}
}

void AMasterHWCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMasterHWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMasterHWCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMasterHWCharacter::Look);

		//에디터에서 IA_Fire 에셋을 할당해야 함.
		if (FireAction)
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AMasterHWCharacter::Fire);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AMasterHWCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMasterHWCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMasterHWCharacter::Fire()
{
	if (!CurrentWeapon) return;

	//카메라 위치에서 카메라 정면 방향으로 Trace
	FVector StartPos = FollowCamera->GetComponentLocation();
	FVector Direction = FollowCamera->GetForwardVector();

	CurrentWeapon->Fire(GetController(), StartPos, Direction);

	//반동 적용
	float Pitch = CurrentWeapon->RecoilPitch;
	float Yaw = FMath::RandRange(-CurrentWeapon->RecoilYawRange, CurrentWeapon->RecoilYawRange);

	//음수 Pitch => 조준선이 위로
	AddControllerPitchInput(-Pitch);
	AddControllerYawInput(Yaw);

	//회복할 량 기록 (Tick에서 되돌림)
	RecoilAccum.X += Pitch;
	RecoilAccum.Y += Yaw;
}

float AMasterHWCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP = FMath::Max(0.f, CurrentHP - ActualDamage);

	UE_LOG(LogTemplateCharacter, Warning,
		TEXT("[%s] %.1f 데미지 | HP: %.1f / %.1f"),
		*GetName(), ActualDamage, CurrentHP, MaxHP);

	if (CurrentHP <= 0.f)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[%s] 사망"), *GetName());
	}

	return ActualDamage;
}
