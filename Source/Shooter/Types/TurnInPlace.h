// Hello :)

#pragma once

UENUM(BlueprintType)
enum class ETurnInPlace : uint8
{
	ETurnIP_Left		UMETA(DisplayName = "TurnLeft"),
	ETurnIP_Right		UMETA(DisplayName = "TurnRight"),
	ETurnIP_NotTurning  UMETA(DisplayName = "NotTurning"),

	ETurnIP_Max			UMETA(DisplayName = "DefaultMax")
	
	
};
