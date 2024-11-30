// |------------------------------------|
// |  !FOLLOW URBAN SHADOWS ON SOCIALS! |
// |------------------------------------|
//            \ (^ᴗ^) /
//             \     /
//              -----
//              |   |
//              |   |
//              |_  |_
////////////////////////////////////////////////////////////
// STEAM P2P SESSIONS - Unreal 5.5 + Steamworks SDK 1.57 //
//////////////////////////////////////////////////////////
#include "SteamInitializer.h"
#include "steam/steam_api.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Net/UnrealNetwork.h"
#include "SteamPlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
bool bSteamInitialized = false;
//////////////////////////////////////////
// 1. Steam Initialization / Additions //
/////////////////////////////////////////////
// -1.1 Steam initializer                 //
// -1.2 Steam shutdown                   //
// -1.3 Check if steam is initialized   //
// -1.4 Register the player to session //
////////////////////////////////////////
// - 1.1 - //
void USteamInitializer::InitializeSteam()
{
    // Prevent reinitialization if Steam is already initialized
    if (bSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Steam is already initialized. Skipping reinitialization."));
        return;
    }

    // Ensure the OnlineSubsystem is available
    IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
    if (!OnlineSubsystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is unavailable! Make sure it is configured correctly in your project settings."));
        return;
    }

    // Attempt to initialize the Steam API
    if (SteamAPI_Init())
    {
        bSteamInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("Steam API successfully initialized."));

        // Log details about the local Steam user (optional)
        if (SteamUser())
        {
            FString SteamUsername = UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
            uint64 SteamID = SteamUser()->GetSteamID().ConvertToUint64();
            UE_LOG(LogTemp, Log, TEXT("Logged into Steam as: %s (SteamID: %llu)"), *SteamUsername, SteamID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SteamUser or SteamFriends is unavailable. Steam user details will not be accessible."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Steam API! Ensure Steam is running, and the project is properly configured for Steam."));
    }
}
// - 1.2 - //
void USteamInitializer::ShutdownSteam()
{
    SteamAPI_Shutdown();
}
// - 1.3 - //
bool USteamInitializer::IsSteamInitialized() const
{
    return SteamAPI_IsSteamRunning();
}
// - 1.4 -//
void USteamInitializer::RegisterPlayer(FName SessionName)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not available! Cannot register player."));
        return;
    }

    IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Session interface is not valid! Cannot register player."));
        return;
    }

    ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is null! Cannot register player."));
        return;
    }

    FUniqueNetIdRepl PlayerId = LocalPlayer->GetPreferredUniqueNetId();
    if (!PlayerId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerId is invalid! Cannot register player."));
        return;
    }

    // Register the local player with the session
    if (SessionInterface->RegisterPlayer(SessionName, *PlayerId.GetUniqueNetId(), false))
    {
        UE_LOG(LogTemp, Log, TEXT("Player registered successfully for session: %s"), *SessionName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register player for session: %s"), *SessionName.ToString());
    }
}
/////////////////////////////
// 2. Pre-lobby mechanics //
//////////////////////////////////////////
// -2.1 Hosting/Creating Session       //
// -2.2 Session Create Callback       //    
// -2.3 Find Games                   //
// -2.4 Sessions search callback    //
// -2.5 Array of found sessions    //
// -2.6 Join the specified game   //
// -2.7 Join game callback       //
//////////////////////////////////
// - 2.1 - //
void USteamInitializer::HostServer(int32 MaxPlayers, FString TargetMap)
{
    IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
    if (OnlineSubsystemInstance)
    {
        // Create session settings
        FOnlineSessionSettings SessionSettings;
        SessionSettings.NumPublicConnections = MaxPlayers; // Number of players allowed
        SessionSettings.NumPrivateConnections = 0; // Private slots (if any)
        SessionSettings.bIsLANMatch = false; // Use Steam instead of LAN
        SessionSettings.bShouldAdvertise = true; // Make the session visible
        SessionSettings.bUsesPresence = true; // Enable presence for Steam
        SessionSettings.bUseLobbiesIfAvailable = true; // Use Steam lobbies if possible
        SessionSettings.bAllowJoinInProgress = true; // Allow players to join mid-game
        SessionSettings.bAllowJoinViaPresence = true; // Allow joining via friends list
        SessionSettings.bIsDedicated = false; // Peer-to-peer setup

        // Add custom key-value pair for filtering
        SessionSettings.Set(FName("GameKey"), FString("urbanshadows"), EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings.Set(FName("MapName"), TargetMap, EOnlineDataAdvertisementType::ViaOnlineService);
        // Set the unique session ID (or game key)
        FString UniqueSessionID = FGuid::NewGuid().ToString();
        SessionSettings.Set(FName("GameSessionID"), UniqueSessionID, EOnlineDataAdvertisementType::ViaOnlineService);
        GameSessionID = UniqueSessionID;
        // Session Name
        FString UniqueSessionName = FGuid::NewGuid().ToString();
        FName SessionName(*UniqueSessionName);
        SessionSettings.Set(FName("SESSION_NAME"), UniqueSessionName, EOnlineDataAdvertisementType::ViaOnlineService);
        RandomName = UniqueSessionName;


        IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            // Bind the callback for session creation completion
            SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
                FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamInitializer::OnCreateSessionComplete)
            );

            // Attempt to create the session
            bool bSessionCreated = SessionInterface->CreateSession(0, SessionName, SessionSettings);
            if (!bSessionCreated)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to initiate CreateSession."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is not available!"));
    }
}
// - 2.2 - //
void USteamInitializer::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session created successfully: %s"), *SessionName.ToString());

        //Register player
        RegisterPlayer(SessionName);

        // Player is a host
        bIsHost = true;

        // Add the host to the lobby
        FPlayerLobbyInfo HostPlayer;
        HostPlayer.PlayerName = TEXT("HostName");  // Fetch from Steam API
        HostPlayer.PlayerAvatar = nullptr;  // Placeholder for now
        HostPlayer.bIsReady = false;

        PlayerLobbyInfoArray.Add(HostPlayer);

        IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
        if (OnlineSubsystemInstance)
        {
            IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                FString MapName;
                if (IOnlineSubsystem::Get()->GetSessionInterface()->GetSessionSettings(SessionName)->Get(FName("MapName"), MapName))
                {
                    GetWorld()->ServerTravel(MapName);
                    UE_LOG(LogTemp, Warning, TEXT("Map traveled: %s"), *MapName);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to travel to map"));
                }
            }
            else
            {
                 UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is not available!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());
    }
}
// - 2.3 - //
void USteamInitializer::FindGames(int32 numberOfGames, FString InGameSessionID, bool UsingGameSessionID)
{
    IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
    if (OnlineSubsystemInstance)
    {
        IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            // Create search settings for the session search
            CurrentSearchSettings = MakeShared<FOnlineSessionSearch>();
            CurrentSearchSettings->MaxSearchResults = numberOfGames;
            CurrentSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

            // Clear previous results
            FoundSessions.Empty();

            // Add a custom filter to search for sessions with the key "urbanshadows"
            CurrentSearchSettings->QuerySettings.Set(FName("GameKey"), FString("urbanshadows"), EOnlineComparisonOp::Equals);
            if (UsingGameSessionID)
            {
                CurrentSearchSettings->QuerySettings.Set(FName("GameSessionID"), InGameSessionID, EOnlineComparisonOp::Equals);
            }

            // Start the session search
            bool bSearchStarted = SessionInterface->FindSessions(0, CurrentSearchSettings.ToSharedRef());

            if (bSearchStarted)
            {
                // Bind the callback for when the session search completes
                SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
                    FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamInitializer::OnFindSessionsComplete)
                );
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to start session search."));
            }
        }
    }
}
// - 2.4 - //
void USteamInitializer::OnFindSessionsComplete(bool bSuccess)
{
    if (bSuccess && CurrentSearchSettings.IsValid())
    {
        FoundSessions = CurrentSearchSettings->SearchResults;
        UE_LOG(LogTemp, Warning, TEXT("Found %d sessions."), FoundSessions.Num());

        // Filter sessions by their state (only keep InProgress sessions)
        TArray<FOnlineSessionSearchResult> InProgressSessions;

        IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
        if (OnlineSubsystemInstance)
        {
            IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                for (const auto& Session : FoundSessions)
                {
                    // Check session state
                    FString temp;
                    Session.Session.SessionSettings.Get(FName("SESSION_NAME"), temp);
                    EOnlineSessionState::Type SessionState = SessionInterface->GetSessionState((*temp));

                    if (SessionState == EOnlineSessionState::InProgress)
                    {
                        // Only add sessions that are in the "InProgress" state
                        FString GameKey;
                        if (Session.Session.SessionSettings.Get(FName("GameKey"), GameKey))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Session found with GameKey: %s"), *GameKey);

                            // Optionally check if this session belongs to the host
                            if (Session.Session.OwningUserId.IsValid())
                            {
                                FString HostName = Session.Session.OwningUserId->ToString();
                                UE_LOG(LogTemp, Warning, TEXT("Session host: %s"), *HostName);
                                // Set bIsHost based on the player's role (if needed)
                                bIsHost = false; // Assuming the client is joining and not hosting
                            }
                        }
                        InProgressSessions.Add(Session);
                    }
                }
            }
        }

        // Log filtered sessions
        UE_LOG(LogTemp, Warning, TEXT("Found %d InProgress sessions."), InProgressSessions.Num());

        // Optionally, send these sessions to Blueprint
        // You can set the filtered sessions to a Blueprint variable or process them further here

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FindSessions failed."));
    }
}
// - 2.5 - //
TArray<FCustomBlueprintSessionResult> USteamInitializer::GetFoundSessions() const
{
    TArray<FCustomBlueprintSessionResult> BlueprintResults;

    // Loop through each found session
    for (const auto& Session : FoundSessions)
    {
        FCustomBlueprintSessionResult Result;

        // Set session details
        Result.OwnerName = Session.Session.OwningUserName;
        Result.MaxPlayers = Session.Session.SessionSettings.NumPublicConnections;
        Result.CurrentPlayers = Result.MaxPlayers - Session.Session.NumOpenPublicConnections;
        Session.Session.SessionSettings.Get(FName("SESSION_NAME"), Result.SessionName);

        // Add the result to the array
        BlueprintResults.Add(Result);
    }

    return BlueprintResults;
}
// - 2.6 - //
void USteamInitializer::JoinGame(int32 SessionIndex, FName SessionName)
{
    IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
    if (!OnlineSubsystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is not available!"));
        return;
    }
    else
    {

        IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
        if (!SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
            return;
        }
        else
        {
            if (!FoundSessions.IsValidIndex(SessionIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid session index: %d"), SessionIndex);
                return;
            }
            else
            {
                const FOnlineSessionSearchResult& SessionToJoin = FoundSessions[SessionIndex];
                FoundSessions[SessionIndex].Session.SessionSettings.bUsesPresence = true; // Enable presence for Steam
                FoundSessions[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true; // Use Steam lobbies if possible

                // Log session details for debugging
                UE_LOG(LogTemp, Warning, TEXT("Joining Session %d: Owner = %s, Ping = %d ms, Players = %d/%d"),
                    SessionIndex,
                    *SessionToJoin.Session.OwningUserName,
                    SessionToJoin.PingInMs,
                    SessionToJoin.Session.SessionSettings.NumPublicConnections - SessionToJoin.Session.NumOpenPublicConnections,
                    SessionToJoin.Session.SessionSettings.NumPublicConnections);
                // Retrieve the unique GameSessionName
                if (SessionToJoin.Session.SessionSettings.Get(FName("SESSION_NAME"), RandomName))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found session with Session Name: %s"), *RandomName);
                }
                // Retrieve the unique GameSessionID
                if (SessionToJoin.Session.SessionSettings.Get(FName("GameSessionID"), GameSessionID))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found session with GameSessionID: %s"), *GameSessionID);
                }

                // Bind the delegate to handle completion of the join session request
                SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
                    FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamInitializer::OnJoinSessionComplete)
                );

                // Attempt to join the session
                if (!SessionInterface->JoinSession(0, NAME_GameSession, SessionToJoin))
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to initiate JoinSession for index: %d"), SessionIndex);
                }
            }
        }
    }
}
// - 2.7 - //
void USteamInitializer::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully joined session: %s"), *SessionName.ToString());

        // Register player
        RegisterPlayer(SessionName);

        // Example: Add a new player to the lobby array
        FPlayerLobbyInfo NewPlayer;
        NewPlayer.PlayerName = TEXT("Guest");  // Fetch from Steam API or session details
        NewPlayer.PlayerAvatar = nullptr;  // Placeholder for now
        NewPlayer.bIsReady = false;

        PlayerLobbyInfoArray.Add(NewPlayer);
        BroadcastLobbyUpdate();

        // Retrieve the connection string
        IOnlineSubsystem* OnlineSubsystemInstance = IOnlineSubsystem::Get();
        if (OnlineSubsystemInstance)
        {
            IOnlineSessionPtr SessionInterface = OnlineSubsystemInstance->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                FString ConnectInfo;
                if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Connect Info: %s"), *ConnectInfo);

                    // Travel to the session
                    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
                    if (PlayerController)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Client traveling to session..."));
                        PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
                    }

                    // Handle session role (client will be joining, so no changes to the session)
                    bIsHost = false; // Mark this player as a client
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to get connection string for session: %s"), *SessionName.ToString());
                }
            }
        }
    }
    else
    {
        // Handle the failure as before
        FString FailureReason;
        switch (Result)
        {
        case EOnJoinSessionCompleteResult::SessionIsFull:
            FailureReason = "Session is full.";
            break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            FailureReason = "Session does not exist.";
            break;
        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
            FailureReason = "Could not retrieve connection address.";
            break;
        default:
            FailureReason = "Unknown error.";
            break;
        }

        UE_LOG(LogTemp, Error, TEXT("Failed to join session: %s, Reason: %s"), *SessionName.ToString(), *FailureReason);
    }
}
////////////////////////////
// 3. In lobby mechanics //
//////////////////////////////////////////////
// -3.1 Updating player info               //
// -3.2 Check if all players are ready    //    
// -3.3 Start game                       //
// -3.4 Ready Up                        //
// -3.5 Broadcast Data                 //
////////////////////////////////////////
// - 3.1 - //
void USteamInitializer::UpdatePlayerInfo(const FString& PlayerName, UTexture2D* PlayerAvatar, bool bIsReady)
{
    FPlayerLobbyInfo PlayerInfo;
    PlayerInfo.PlayerName = PlayerName;
    PlayerInfo.PlayerAvatar = PlayerAvatar;
    PlayerInfo.bIsReady = bIsReady;

    // Update player info in the session or local list
    PlayerLobbyInfoMap.Add(PlayerName, PlayerInfo);
}
// - 3.2 - //
bool USteamInitializer::AreAllPlayersReady() const
{
    for (const auto& Player : PlayerLobbyInfoMap)
    {
        if (!Player.Value.bIsReady)
        {
            return false;
        }
    }
    return true;
}
// - 3.3 - //
void USteamInitializer::StartGame()
{
    if (AreAllPlayersReady())
    {
        // Logic to start the game
        // Transition to the game map or notify everyone
    }
}
// - 3.4 - //
void USteamInitializer::SetPlayerReadyStatus(const FString& PlayerName, bool bIsReady)
{
    for (FPlayerLobbyInfo& Player : PlayerLobbyInfoArray)
    {
        if (Player.PlayerName == PlayerName)
        {
            Player.bIsReady = bIsReady;
            break;
        }
    }
}
// - 3.5 - //
void USteamInitializer::BroadcastLobbyUpdate()
{
    // Iterate over all player controllers in the world
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        // Retrieve the player controller
        APlayerController* BaseController = It->Get();
        if (BaseController)
        {
            // Cast to your custom SteamPlayerController class
            if (ASteamPlayerController* SteamController = Cast<ASteamPlayerController>(BaseController))
            {
                // Call the custom client function to update the lobby
                SteamController->ClientUpdateLobby(PlayerLobbyInfoArray);
            }
        }
    }
}
///////////////////////////////////////////
// MORE COMING SOON  // A.S. - hajddeen //
/////////////////////////////////////////
