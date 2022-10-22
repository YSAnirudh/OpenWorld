// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"

/**
 * 
 */
UCLASS()
class OPENWORLD_API AMainHUD : public AHUD
{
	GENERATED_BODY()
public:
	AMainHUD();

protected:
	virtual void BeginPlay() override;

private:
	
};
