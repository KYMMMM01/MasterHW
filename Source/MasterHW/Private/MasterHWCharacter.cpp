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
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "DELEGATE/HealthComponent.h"
#include "UI/HealthBarWidget.h"

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

	//체력 컴포넌트 부착
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void AMasterHWCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnHealthDead.AddDynamic(this, &AMasterHWCharacter::HandleDeath);
	}

	//로컬 플레이어만 체력바 UI 생성
	if (IsLocallyControlled() && HealthBarClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			HealthBarWidget = CreateWidget<UHealthBarWidget>(PC, HealthBarClass);
			if (HealthBarWidget)
			{
				HealthBarWidget->AddToViewport();
				if (HealthComp)
				{
					HealthComp->OnHealthDamaged.AddDynamic(HealthBarWidget, &UHealthBarWidget::UpdateHealth);
					//초기 체력바 채우기
					HealthBarWidget->UpdateHealth(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth(), 0.f);
				}
			}
		}
	}

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

void AMasterHWCharacter::DebugDamage(float Amount)
{
	//테스트용: 자신에게 데미지 => OnTakeAnyDamage => HealthComponent가 처리
	UGameplayStatics::ApplyDamage(this, Amount, GetController(), this, UDamageType::StaticClass());
}

void AMasterHWCharacter::HandleDeath(AController* DeathInstigator)
{
	//리스폰 대상 컨트롤러 기억
	DeadController = GetController();

	//입력 차단 + 데스 캠(죽는 동안 카메라가 쓰러진 몸을 보도록)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
		PC->SetViewTargetWithBlend(this, 0.3f);
	}

	//이동 정지
	GetCharacterMovement()->DisableMovement();

	//캡슐 콜리전 끄고 메시 래그돌
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	//컨트롤러 분리(리스폰 때 새 폰을 빙의시키기 위해)
	if (DeadController)
	{
		DeadController->UnPossess();
	}

	//일정 시간 후 리스폰
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMasterHWCharacter::Respawn, RespawnDelay, false);
}

void AMasterHWCharacter::Respawn()
{
	//GameMode에 리스폰 요청(PlayerStart에서 새 캐릭터 스폰 후 빙의)
	if (DeadController)
	{
		if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
		{
			GM->RestartPlayer(DeadController);
		}
	}

	//손에 든 무기 제거
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
	}

	//시체(이 액터) 제거
	Destroy();
}

void AMasterHWCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//리스폰/파괴 시 화면에서 위젯 제거 (중복 누적 방지)
	if (HealthBarWidget)
	{
		HealthBarWidget->RemoveFromParent();
		HealthBarWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}
