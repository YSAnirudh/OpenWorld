// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MainCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerCapsule = FindComponentByClass<UCapsuleComponent>();
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(PlayerCapsule);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset.Z = 75.f;
	SpringArm->SocketOffset.Y = 50.f;

	MainCamera = CreateDefaultSubobject<UCameraComponent>("Follow Camera");
	MainCamera->SetupAttachment(SpringArm);
	MainCamera->AddRelativeRotation(FRotator(-100.f, 0.f, 0.f));

	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Controller Missing!!!"));
	}

	CharacterMovementComponent = FindComponentByClass<UCharacterMovementComponent>();
	if (!CharacterMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Character Movement Component Missing!!!"));
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("JogForward"), this, &AMainCharacter::JogForward);
	PlayerInputComponent->BindAxis(TEXT("StrafeRight"), this, &AMainCharacter::StrafeRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMainCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMainCharacter::Turn);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Shoot"), IE_Pressed, this, &AMainCharacter::Shoot);
}

void AMainCharacter::JogForward(float AxisValue)
{
	FRotator ControlYaw {0.0f, PlayerController->GetControlRotation().Yaw, 0.0f};
	FVector Direction {FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X)};
	
	AddMovementInput(Direction, GetWorld()->GetDeltaSeconds() * AxisValue * MoveSpeed);
}

void AMainCharacter::StrafeRight(float AxisValue)
{
	FRotator ControlYaw {0.0f, PlayerController->GetControlRotation().Yaw, 0.0f};
	FVector Direction {FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::Y)};
	
	AddMovementInput(Direction, GetWorld()->GetDeltaSeconds() * AxisValue * MoveSpeed);
}

void AMainCharacter::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue * GetWorld()->GetDeltaSeconds() * Sensitivity);
}

void AMainCharacter::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue * GetWorld()->GetDeltaSeconds() * Sensitivity);
}

void AMainCharacter::Shoot()
{
	if (ShootSound)
	{
		UGameplayStatics::PlaySound2D(this, ShootSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName(TEXT("BarrelSocket"));

	if (BarrelSocket)
	{
		const FTransform BarrelTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlashParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashParticles, BarrelTransform);
		}
		
		FVector BeamEnd;
		bool bBeamEnd = LineTraceShoot(BarrelTransform.GetLocation(), BeamEnd);
		
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, BarrelTransform);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(TEXT("StartFire"));
	}
}

bool AMainCharacter::LineTraceShoot(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FVector2d ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2d CrosshairLocation = ViewportSize/2;
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldLocation;
	FVector CrosshairWorldDirection;
	bool bDeprojectSuccess = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation, CrosshairWorldLocation, CrosshairWorldDirection);

	if (bDeprojectSuccess)
	{
		FHitResult OutHit;
		FVector StartVector = CrosshairWorldLocation;
		OutBeamLocation = CrosshairWorldLocation + CrosshairWorldDirection * ShootDistance;

		GetWorld()->LineTraceSingleByChannel(
			OutHit,
			StartVector,
			OutBeamLocation,
			ECollisionChannel::ECC_Visibility
		);
		if (OutHit.bBlockingHit)
		{
			OutBeamLocation = OutHit.Location;
		}
		FHitResult WeaponTraceHit;
		StartVector = MuzzleSocketLocation;

		GetWorld()->LineTraceSingleByChannel(
			OutHit,
			StartVector,
			OutBeamLocation,
			ECollisionChannel::ECC_Visibility
		);

		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}



