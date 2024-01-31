// Hello :)

#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssualtRifle   UMETA(Display = "AssaultRifle"),
	EWT_RocketLauncher UMETA(Display = "RocketLauncher"),

	EWT_MAX UMETA(Display = "DefaultMAX"),

};