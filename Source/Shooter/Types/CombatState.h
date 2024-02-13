// Hello :) 

#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECState_Unoccupied   UMETA(DisplayName = "Unoccupied"),
	ECState_Reloading    UMETA(DisplayName = "Reloading"),
	ECState_ThrowGrenade UMETA(DisplayName = "ThrowGrenade"),
	ECState_Slide		 UMETA(DisplayName = "Slide"),
	
	ECState_MAX UMETA(DisplayName = "DefaultMax"),

};