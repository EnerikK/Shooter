// Hello :)

#pragma once

#define TRACE 80000.f
#define CUSTOM_DEPTH_PURPLE 250;
#define Custom_Depth_Blue   251;
#define Custom_Depth_Tan    252;
#define Custom_Depth_Red    255;


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssualtRifle     UMETA(Display = "AssaultRifle"),
	EWT_RocketLauncher   UMETA(Display = "RocketLauncher"),
	EWT_Pistol		     UMETA(Display = "Pistol"),
	EWT_SMG		         UMETA(Display = "SMG"),
	EWT_Shotgun		     UMETA(Display = "Shotgun"),
	EWT_Sniper		     UMETA(Display = "Sniper"),
	EWT_GrenadeLauncher	 UMETA(Display = "GrenadeLauncher"),



	
	EWT_MAX UMETA(Display = "DefaultMAX"),

};