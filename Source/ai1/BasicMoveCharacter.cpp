// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicMoveCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Fireball.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogBasicMove, Log, All);

// ============================================================================
// 构造函数
// ============================================================================

ABasicMoveCharacter::ABasicMoveCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 胶囊体
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// 默认不随控制器旋转（由鼠标 Look 控制）
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;
	bUseControllerRotationYaw   = false;

	// 地面移动配置
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
	MoveComp->JumpZVelocity = 600.f;
	MoveComp->AirControl = 0.2f;
	MoveComp->MaxWalkSpeed = 600.f;

	// 多段跳：JumpMaxCount 是 ACharacter 的属性
	JumpMaxCount = 2;

	// 弹簧臂 + 相机
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 3.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 骨骼网格
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}
}

// ============================================================================
// BeginPlay
// ============================================================================

void ABasicMoveCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 通过 PlayerController 注册 IMC（PlayerController 更早初始化，EnhancedInput 子系统更可靠）
	if (bInputAssetsInitialized)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Sub->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogBasicMove, Warning, TEXT("BeginPlay: AddMappingContext OK (direct)."));
			}
		}
	}
}

// ============================================================================
// 输入资产初始化（动态创建 IMC + InputAction）
// ============================================================================

void ABasicMoveCharacter::InitializeInputAssets()
{
	// ---- InputAction ----
	MoveForwardAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_MoveForward"));
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;

	MoveRightAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_MoveRight"));
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;

	// 鼠标 Yaw/Pitch 拆成独立 Axis1D，MouseX/MouseY 在 PIE 下比 Mouse2D 可靠
	LookYawAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_LookYaw"));
	LookYawAction->ValueType = EInputActionValueType::Axis1D;

	LookPitchAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_LookPitch"));
	LookPitchAction->ValueType = EInputActionValueType::Axis1D;

	JumpAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_Jump"));
	JumpAction->ValueType = EInputActionValueType::Boolean;

	FlyAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_Fly"));
	FlyAction->ValueType = EInputActionValueType::Boolean;

	ShootAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), TEXT("IA_Shoot"));
	ShootAction->ValueType = EInputActionValueType::Boolean;

	// ---- InputMappingContext ----
	DefaultMappingContext = NewObject<UInputMappingContext>(this, UInputMappingContext::StaticClass(), TEXT("IMC_Default"));

	// 前后移动：W = +1, S = -1
	DefaultMappingContext->MapKey(MoveForwardAction, EKeys::W);
	{
		FEnhancedActionKeyMapping& S = DefaultMappingContext->MapKey(MoveForwardAction, EKeys::S);
		S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}

	// 左右移动：D = +1, A = -1
	DefaultMappingContext->MapKey(MoveRightAction, EKeys::D);
	{
		FEnhancedActionKeyMapping& A = DefaultMappingContext->MapKey(MoveRightAction, EKeys::A);
		A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}

	// 鼠标 Yaw（左右看）
	DefaultMappingContext->MapKey(LookYawAction, EKeys::MouseX);

	// 鼠标 Pitch（上下看），加 Negate 使鼠标上推 = 抬头
	{
		FEnhancedActionKeyMapping& Pitch = DefaultMappingContext->MapKey(LookPitchAction, EKeys::MouseY);
		Pitch.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}

	// 空格 = 按住飞行
	DefaultMappingContext->MapKey(FlyAction, EKeys::SpaceBar);

	// 左 Shift = 跳跃
	DefaultMappingContext->MapKey(JumpAction, EKeys::LeftShift);

	// 鼠标左键 / E 键 = 发射火球
	DefaultMappingContext->MapKey(ShootAction, EKeys::LeftMouseButton);
	DefaultMappingContext->MapKey(ShootAction, EKeys::E);

	UE_LOG(LogBasicMove, Warning, TEXT("Input assets created: MoveFwd/MoveRht/LookYaw/LookPitch/Jump/Fly/Shoot"));
}

// ============================================================================
// SetupPlayerInputComponent
// ============================================================================

void ABasicMoveCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!bInputAssetsInitialized)
	{
		InitializeInputAssets();
		bInputAssetsInitialized = true;
	}

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogBasicMove, Warning, TEXT("Cast to EnhancedInputComponent FAILED!"));
		return;
	}

	// 前后 / 左右拆成独立的 Axis1D，避免 1D 按键映射到 Axis2D 时的轴向错乱
	EnhancedInput->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ABasicMoveCharacter::Input_MoveForward);
	EnhancedInput->BindAction(MoveRightAction,   ETriggerEvent::Triggered, this, &ABasicMoveCharacter::Input_MoveRight);
	EnhancedInput->BindAction(LookYawAction,     ETriggerEvent::Triggered, this, &ABasicMoveCharacter::Input_LookYaw);
	EnhancedInput->BindAction(LookPitchAction,   ETriggerEvent::Triggered, this, &ABasicMoveCharacter::Input_LookPitch);
	EnhancedInput->BindAction(JumpAction,        ETriggerEvent::Started,   this, &ABasicMoveCharacter::Input_JumpPressed);
	EnhancedInput->BindAction(JumpAction,        ETriggerEvent::Completed, this, &ABasicMoveCharacter::Input_JumpReleased);
	EnhancedInput->BindAction(FlyAction,         ETriggerEvent::Started,   this, &ABasicMoveCharacter::Input_FlyPressed);
	EnhancedInput->BindAction(FlyAction,         ETriggerEvent::Completed, this, &ABasicMoveCharacter::Input_FlyReleased);
	EnhancedInput->BindAction(ShootAction,       ETriggerEvent::Started,   this, &ABasicMoveCharacter::Input_Shoot);

	UE_LOG(LogBasicMove, Warning, TEXT("All 6 actions bound to EnhancedInputComponent — IMC will be added in BeginPlay via PlayerController"));
}

// ============================================================================
// 移动
// ============================================================================

void ABasicMoveCharacter::Input_MoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (!Controller || FMath::IsNearlyZero(Axis)) return;

	if (bIsFlying)
	{
		FlightInputAxis.X = Axis;  // 飞行时不走 AddMovementInput，自己记录
	}
	else
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), Axis);
	}
}

void ABasicMoveCharacter::Input_MoveRight(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (!Controller || FMath::IsNearlyZero(Axis)) return;

	if (bIsFlying)
	{
		FlightInputAxis.Y = Axis;  // 飞行时不走 AddMovementInput，自己记录
	}
	else
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), Axis);
	}
}

// ============================================================================
// 视角旋转
// ============================================================================

void ABasicMoveCharacter::Input_LookYaw(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	PendingLookDelta.X += Axis;
}

void ABasicMoveCharacter::Input_LookPitch(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	PendingLookDelta.Y += Axis;
}

// ============================================================================
// 跳跃（使用 ACharacter 内置跳跃系统，支持多段跳）
// ============================================================================

void ABasicMoveCharacter::Input_JumpPressed()
{
	Jump();
}

void ABasicMoveCharacter::Input_JumpReleased()
{
	StopJumping();
}

// ============================================================================
// Tick
// ============================================================================

void ABasicMoveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 在 Tick 中统一应用鼠标视角旋转（避免 Input 回调与 PlayerController::ProcessPlayerInput 的时序冲突）
	if (!PendingLookDelta.IsNearlyZero() && Controller)
	{
		FRotator CR = Controller->GetControlRotation();
		CR.Yaw   += PendingLookDelta.X;
		CR.Pitch  = FMath::Clamp(CR.Pitch + PendingLookDelta.Y, -89.9f, 89.9f);
		CR.Roll   = 0.f;
		Controller->SetControlRotation(CR);
		PendingLookDelta = FVector2D::ZeroVector;
	}

	if (bIsFlying)
	{
		MaintainFlight(DeltaTime);
	}
}

// ============================================================================
// 攻击
// ============================================================================

void ABasicMoveCharacter::Input_Shoot()
{
	if (!Controller) return;

	// 发射方向：角色面朝方向
	const FVector ShootDir = GetActorForwardVector();
	const FVector SpawnLoc = GetActorLocation() + ShootDir * 100.f + FVector(0.f, 0.f, 50.f);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFireball* Ball = GetWorld()->SpawnActor<AFireball>(AFireball::StaticClass(), SpawnLoc, ShootDir.Rotation(), Params);
	if (Ball)
	{
		Ball->FireInDirection(ShootDir);
	}
}

// ============================================================================
// 飞行
// ============================================================================

void ABasicMoveCharacter::Input_FlyPressed()
{
	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC || bIsFlying) return;

	bIsFlying = true;

	// 切到 MOVE_Custom 模式，彻底绕过 PhysFalling/PhysFlying
	MC->SetMovementMode(MOVE_Custom, 1);
	MC->Velocity = FVector::ZeroVector;
	MC->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	UE_LOG(LogBasicMove, Warning, TEXT("Fly ON (Custom mode)"));
}

void ABasicMoveCharacter::Input_FlyReleased()
{
	if (!bIsFlying) return;

	bIsFlying = false;
	FlightInputAxis = FVector2D::ZeroVector;

	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (MC)
	{
		MC->SetMovementMode(MOVE_Falling);
		MC->bOrientRotationToMovement = true;
	}

	bUseControllerRotationYaw = false;

	UE_LOG(LogBasicMove, Warning, TEXT("Fly OFF  falling"));
}

void ABasicMoveCharacter::MaintainFlight(float DeltaTime)
{
	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC || !bIsFlying) return;

	// 用 FlightInputAxis + 相机方向 计算世界空间目标方向
	const FVector2D Axis = FlightInputAxis;
	FVector WorldDir = FVector::ZeroVector;

	if (!Axis.IsNearlyZero() && Controller)
	{
		const FRotator CR = Controller->GetControlRotation();
		WorldDir += FRotationMatrix(CR).GetUnitAxis(EAxis::X) * Axis.X;  // 前后
		WorldDir += FRotationMatrix(CR).GetUnitAxis(EAxis::Y) * Axis.Y;  // 左右
		WorldDir.Normalize();
	}

	// 目标速度
	const FVector TargetVel = WorldDir * FlyingMaxSpeed;

	// 平滑插值
	MC->Velocity = FMath::VInterpTo(MC->Velocity, TargetVel, DeltaTime, FlyingInterpSpeed);

	// 自己移动（MOVE_Custom 不会自动移动）
	FHitResult Hit;
	MC->SafeMoveUpdatedComponent(MC->Velocity * DeltaTime, GetCapsuleComponent()->GetComponentQuat(), true, Hit);

	if (Hit.bBlockingHit)
	{
		// 简单的沿法线滑动
		MC->Velocity = FVector::VectorPlaneProject(MC->Velocity, Hit.Normal);
	}
}
