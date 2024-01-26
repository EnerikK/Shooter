// Hello :) 

#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECState_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECState_Reloading  UMETA(DisplayName = "Reloading"),
	
	ECState_MAX UMETA(DisplayName = "DefaultMax"),

};