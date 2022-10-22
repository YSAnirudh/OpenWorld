// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/MainChracterAnimInst.h"

#include "Kismet/KismetMathLibrary.h"

void UMainChracterAnimInst::UpdateAnimationProperties(float DeltaTime)
{
	if (!MainChracterRef)
	{
		MainChracterRef = Cast<AMainCharacter>(TryGetPawnOwner());
	}

	if (MainChracterRef)
	{
		FVector Velocity{MainChracterRef->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();
		bIsInAir = MainChracterRef->GetCharacterMovement()->IsFalling();
		if (MainChracterRef->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsJogging = true;
		} else
		{
			bIsJogging = false;
		}

		FRotator AimRotation = MainChracterRef->GetBaseAimRotation();

		FRotator CharacterMovement = UKismetMathLibrary::MakeRotFromX(MainChracterRef->GetVelocity());

		OffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, CharacterMovement).Yaw;
	}
}

void UMainChracterAnimInst::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MainChracterRef = Cast<AMainCharacter>(TryGetPawnOwner());

	
}
