// Hello :) 


#include "Shooter/Public/Player/ShooterPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/ShooterCharacter.h"
#include "Components/CombatComponent.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Game/ShooterGameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "GameState/ShooterGameState.h"
#include "Hud/Announcement.h"
#include "Hud/HudOverlay.h"
#include "Hud/ReturnToMainMenu.h"
#include "Hud/ShooterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/ShooterPlayerState.h"
#include "Shooter/Types/Announcement.h"

AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
}
void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AShooterPlayerController,MatchState);
	DOREPLIFETIME(AShooterPlayerController,bShowTeamScores);
	DOREPLIFETIME(AShooterPlayerController,bDisableGameplay);
}
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(PlayerContext,0);
	}
	
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	ServerCheckMatchState();
}
void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHudTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
	
}
void AShooterPlayerController::BroadcastKill(APlayerState* Attacker, APlayerState* Victim)
{
	ClientKillAnnouncement(Attacker,Victim);
}
void AShooterPlayerController::ClientKillAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if(Attacker && Victim && Self)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
		if(ShooterHUD)
		{
			if(Attacker == Self && Victim != Self)
			{
				ShooterHUD->AddKillAnnouncement("You",Victim->GetPlayerName());
				return;
			}
			if(Victim == Self && Attacker != Self)
			{
				ShooterHUD->AddKillAnnouncement(Attacker->GetPlayerName(),"you");
				return;
			}
			if(Attacker == Victim && Attacker == Self)
			{
				ShooterHUD->AddKillAnnouncement("You","Yourself");
				return;
			}
			if(Attacker == Victim && Attacker != Self)
			{
				ShooterHUD->AddKillAnnouncement(Attacker->GetPlayerName(),"Themselves");
				return;
			}
			ShooterHUD->AddKillAnnouncement(Attacker->GetPlayerName(),Victim->GetPlayerName());
		}
	}
}
void AShooterPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if(HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<AShooterPlayerState>()  : PlayerState;
		if(PlayerState)
		{
			UE_LOG(LogTemp,Warning,TEXT("PlayerState->GetPing() * 4 : %d"),PlayerState->GetCompressedPing()*4);
			if(PlayerState->GetCompressedPing() * 4 > HighPingThreshold) // Ping is compressed  thats why we * 4 
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if(ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->HighPingAnimation && ShooterHUD->HudOverlay->IsAnimationPlaying(ShooterHUD->HudOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if(PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AShooterPlayerController::OnRep_ShowTeamScores()
{
	if(bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}
void AShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)//Is the ping too high ? 
{
	HighPingDelegate.Broadcast(bHighPing);
}
void AShooterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
 	{
 		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
 		TimeSyncRunningTime = 0.f;
 	}
}
void AShooterPlayerController::PollInit()
{
	if(HudOverlay == nullptr)
	{
		if(ShooterHUD && ShooterHUD->HudOverlay)
		{
			HudOverlay = ShooterHUD->HudOverlay;
			if(HudOverlay)
			{
				if(bInitializeHealth) SetHudHealth(HudHealth,HudMaxHealth);
				if(bInitializeShield) SetHudShield(HudShield,HudMaxShield);
				if(bInitializeScore)  SetHudScore(HudScore);
				if(bInitializeDefeats)SetHudDefeats(HudDefeats);
				if(bInitializeCarriedAmmo) SetHudCarriedAmmo(HudCarriedAmmo);
				if(bInitializeWeaponAmmo) SetHudWeaponAmmo(HudWeaponAmmo);
				AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
				if(ShooterCharacter && ShooterCharacter->GetCombat())
				{
					if(bInitializeGrenades) SetHudGrenades(ShooterCharacter->GetCombat()->GetGrenades());                        
				}
			}
		}
	}
}
void AShooterPlayerController::HighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->HighPingImage && ShooterHUD->HudOverlay->HighPingAnimation;
	if(bHudValid)
	{
		ShooterHUD->HudOverlay->HighPingImage->SetOpacity(1.f);
		ShooterHUD->HudOverlay->PlayAnimation(ShooterHUD->HudOverlay->HighPingAnimation,0.5f,5);
	}
}
void AShooterPlayerController::StopHighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->HighPingImage && ShooterHUD->HudOverlay->HighPingAnimation;
	if(bHudValid)
	{
		ShooterHUD->HudOverlay->HighPingImage->SetOpacity(0.f);
		if(ShooterHUD->HudOverlay->IsAnimationPlaying(ShooterHUD->HudOverlay->HighPingAnimation))
		{
			ShooterHUD->HudOverlay->StopAnimation(ShooterHUD->HudOverlay->HighPingAnimation);
		}
	}
}
void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
	AShooterGameModeBase* GameMode = Cast<AShooterGameModeBase>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		InterventionTime = GameMode->InterventionTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState,CooldownTime,InterventionTime,MatchTime,LevelStartingTime);
	}
}
void AShooterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch,float Cooldown,float Intervention,float Match,float StartingTime)
{
	InterventionTime = Intervention;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if(ShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		ShooterHUD->AddAnnouncement();
	}
}
void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if(ShooterCharacter)
	{
		SetHudHealth(ShooterCharacter->GetHealth(),ShooterCharacter->GetMaxHealth());
	}
	
}
void AShooterPlayerController::SetHudHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->HealthBar && ShooterHUD->HudOverlay->HealthText;
	if(bHudValid)
	{
		const float HealthPercent = Health/MaxHealth;
		ShooterHUD->HudOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
		ShooterHUD->HudOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HudHealth = Health;
		HudMaxHealth = MaxHealth;
	}
}
void AShooterPlayerController::SetHudShield(float Shield, float MaxShield)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->ShieldBar && ShooterHUD->HudOverlay->ShieldText;
	if(bHudValid)
	{
		const float ShieldPercent = Shield/MaxShield;
		ShooterHUD->HudOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Shield),FMath::CeilToInt(MaxShield));
		ShooterHUD->HudOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HudShield = Shield;
		HudMaxShield = MaxShield;
	}
}
void AShooterPlayerController::SetHudScore(float Score)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->ScoreAmount;
	if(bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		ShooterHUD->HudOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HudScore = Score;
	}
}
void AShooterPlayerController::SetHudDefeats(int32 Defeats)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->DefeatsAmount;
	if(bHudValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);
		ShooterHUD->HudOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
		
	}
	else
	{
		bInitializeDefeats = true;
		HudDefeats = Defeats;
	}
}
void AShooterPlayerController::SetHudWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->WeaponAmmoAmount;
	if(bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		ShooterHUD->HudOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		
	}
	else
	{
		bInitializeWeaponAmmo = true;
	}
	
}
void AShooterPlayerController::SetHudCarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->CarriedAmmo;
	if(bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		ShooterHUD->HudOverlay->CarriedAmmo->SetText(FText::FromString(AmmoText));
		
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HudCarriedAmmo = Ammo;
	}
}
void AShooterPlayerController::SetHudMatchCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->MatchCountdownText;
	if(bHudValid)
	{
		if(CountdownTime < 0.f)
		{
			ShooterHUD->HudOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		ShooterHUD->HudOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
		
	}
}
void AShooterPlayerController::SetHudAnnouncementCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->Announcement && ShooterHUD->Announcement->InterventionTime;
	if(bHudValid)
	{
		if(CountdownTime < 0.f)
		{
			ShooterHUD->Announcement->InterventionTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		ShooterHUD->Announcement->InterventionTime->SetText(FText::FromString(CountdownText));
	}
	
}
void AShooterPlayerController::SetHudGrenades(int32 Grenades)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->GrenadesText;
	if(bHudValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"),Grenades); 
		ShooterHUD->HudOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
		
	}
	else
	{
		bInitializeGrenades = true;
		HudGrenades = Grenades;
	}
	
}
void AShooterPlayerController::SetHudRedTeamScore(int32 RedScore)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->RedTeamScore;
	if(bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),RedScore);
		ShooterHUD->HudOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}
void AShooterPlayerController::SetHudBlueTeamScore(int32 BlueScore)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->BlueTeamScore;
	if(bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),BlueScore);
		ShooterHUD->HudOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}
void AShooterPlayerController::HideTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->RedTeamScore && ShooterHUD->HudOverlay->BlueTeamScore && ShooterHUD->HudOverlay->ScoreSpacerText;
	if(bHudValid)
	{
		ShooterHUD->HudOverlay->RedTeamScore->SetText(FText());
		ShooterHUD->HudOverlay->BlueTeamScore->SetText(FText());
		ShooterHUD->HudOverlay->ScoreSpacerText->SetText(FText());
	}
}
void AShooterPlayerController::InitTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->RedTeamScore && ShooterHUD->HudOverlay->BlueTeamScore && ShooterHUD->HudOverlay->ScoreSpacerText;
	if(bHudValid)
	{
		FString Zero("0");
		FString Spacer("|");
		ShooterHUD->HudOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->HudOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->HudOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}
float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
	
}
void AShooterPlayerController::OnMatchStateSet(FName State,bool bTeamsMatch)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void AShooterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void AShooterPlayerController:: HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if(ShooterHUD)
	{
		if(ShooterHUD->HudOverlay == nullptr) ShooterHUD->AddHudOverlay();
		if(ShooterHUD->Announcement)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if(!HasAuthority()) return;
		if(bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}
void AShooterPlayerController::HandleCooldown()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if(ShooterHUD)
	{
		ShooterHUD->HudOverlay->RemoveFromParent();
		if(ShooterHUD->Announcement && ShooterHUD->Announcement->AnnouncementText && ShooterHUD->Announcement->InfoText)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
			
			if(ShooterGameState && ShooterPlayerState)
			{
				TArray<AShooterPlayerState*> TopPlayers = ShooterGameState->TopScoringPlayer;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(ShooterGameState) : GetInfoText(TopPlayers) ;
				
				ShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	bDisableGameplay = true;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}
FString AShooterPlayerController::GetInfoText(const TArray<AShooterPlayerState*>& Players)
{
	AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
	if(ShooterPlayerState == nullptr) return FString();
	FString InfoTextString;
	if(Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == ShooterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner : \n%s"),*Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%p\n"),*TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}
FString AShooterPlayerController::GetTeamsInfoText(AShooterGameState* ShooterGameState)
{
	if(ShooterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = ShooterGameState->RedTeamScore;
	const int32 BlueTeamScore = ShooterGameState->BlueTeamScore;

	if(RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"),*Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d"),*Announcement::RedTeam,RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d"),*Announcement::BlueTeam,BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d"),*Announcement::BlueTeam,BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d"),*Announcement::RedTeam,RedTeamScore));
	}
	return InfoTextString;
}
void AShooterPlayerController::SetHudTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart) TimeLeft = InterventionTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = InterventionTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown) TimeLeft = CooldownTime+ InterventionTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if(HasAuthority())
	{
		ShooterGameMode = ShooterGameMode == nullptr ? Cast<AShooterGameModeBase>(UGameplayStatics::GetGameMode(this)) : ShooterGameMode;
		if(ShooterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(ShooterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
   {
			SetHudAnnouncementCountdown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHudMatchCountdown(TimeLeft);
		}
	}
	
	CountdownInt = SecondsLeft;
	
}
void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceive);
}
void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = (0.5f * RoundTripTime);
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
	
}
void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	EnhancedInputComponent->BindAction(
	MoveAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Move);
		
	EnhancedInputComponent->BindAction(
	LookAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Look);
		
	EnhancedInputComponent->BindAction(
	JumpAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Jump);

	EnhancedInputComponent->BindAction(
	EquipAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Equip);

	EnhancedInputComponent->BindAction(
	CrouchAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Crouch);
	
	EnhancedInputComponent->BindAction(
	AimAction,ETriggerEvent::Started,this,&AShooterPlayerController::Aim);

	EnhancedInputComponent->BindAction(
	ReleaseAimAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::ReleaseAim);

	EnhancedInputComponent->BindAction(
	FireAction,ETriggerEvent::Started,this,&AShooterPlayerController::Fire);

	EnhancedInputComponent->BindAction(
	FireAction,ETriggerEvent::Completed,this,&AShooterPlayerController::ReleaseFire);

	EnhancedInputComponent->BindAction(
	ReloadAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Reload);

	EnhancedInputComponent->BindAction(
	GrenadeToss,ETriggerEvent::Triggered,this,&AShooterPlayerController::Toss);
	
	EnhancedInputComponent->BindAction(
	SlideAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Slide);

	EnhancedInputComponent->BindAction(
	QuitAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Quit);
	
	EnhancedInputComponent->BindAction(
	ShiftPressed,ETriggerEvent::Started,this,&AShooterPlayerController::StartSprint);
	EnhancedInputComponent->BindAction(
	ShiftPressed,ETriggerEvent::Completed,this,&AShooterPlayerController::StopSprint);

	EnhancedInputComponent->BindAction(
	AltPressed,ETriggerEvent::Triggered,this,&AShooterPlayerController::Dash);
	
}

void AShooterPlayerController::Move(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn<APawn>();
	
	if (ControlledPawn != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, ControlledPawn->GetControlRotation().Yaw, 0);
 
		// Forward/Backward direction
		if (MoveValue.Y != 0.f)
		{
			// Get forward vector
			const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			
			ControlledPawn->AddMovementInput(Direction, MoveValue.Y);
		}
 
		// Right/Left direction
		if (MoveValue.X != 0.f)
		{
			// Get right vector
			const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
 
			ControlledPawn->AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void AShooterPlayerController::Look(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn<APawn>();
	
	if (ControlledPawn->Controller != nullptr)
	{
		const FVector2D LookValue = Value.Get<FVector2D>();
 
		if (LookValue.X != 0.f)
		{
			ControlledPawn->AddControllerYawInput(LookValue.X);
			
		}
 
		if (LookValue.Y != 0.f)
		{
			ControlledPawn->AddControllerPitchInput(LookValue.Y);
		}
	}
}
void AShooterPlayerController::Jump(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	if(ACharacter* ControlledCharacter = GetPawn<ACharacter>())
	{
		ControlledCharacter->Jump();
	}
}
void AShooterPlayerController::Equip(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->EquipButtonPressed();
	}
	
}
void AShooterPlayerController::Crouch(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->CrouchButtonPressed();
	}
	
	
}
void AShooterPlayerController::Aim(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->AimButtonPressed();
	}
	
}
void AShooterPlayerController::ReleaseAim(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->AimButtonReleased();
	}
	
}
void AShooterPlayerController::Fire(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->FireButtonPressed();
	}
	
}

void AShooterPlayerController::ReleaseFire(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->FireButtonReleased();
	}
}

void AShooterPlayerController::Reload(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->ReloadButtonPressed();
	}
	
}

void AShooterPlayerController::Toss(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;

	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	if(ControlledCharacter)
	{
		ControlledCharacter->GrenadeButtonPressed();
	}
}

void AShooterPlayerController::Slide(const FInputActionValue& Value)
{
	if(bDisableGameplay) return;
	if(AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter()))
	{
		ControlledCharacter->GetShooterCharacterComponent()->SlidePressed();
	}

}

void AShooterPlayerController::Quit(const FInputActionValue& Value)
{
	if(ReturnToMainMenuWidget == nullptr) return;
	if(ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this,ReturnToMainMenuWidget);
	}
	if(ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetUp();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}
void AShooterPlayerController::StartSprint(const FInputActionValue& Value)
{
	if(AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter()))
	{
		ControlledCharacter->GetShooterCharacterComponent()->SprintPressed();
	}
}
void AShooterPlayerController::StopSprint(const FInputActionValue& Value)
{
	if(AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter()))
	{
		ControlledCharacter->GetShooterCharacterComponent()->SprintReleased();
	}
}

void AShooterPlayerController::Dash(const FInputActionValue& Value)
{
	if(AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter()))
	{
		ControlledCharacter->GetShooterCharacterComponent()->DashPressed();
	}
}
	
