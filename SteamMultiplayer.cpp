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
// HOW TO USE? //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. It is game instance class                                                                       //
// 2. Create BP, In all classes find " SteamMultiplayer "                                            //
// 3. Initialize SteamAPI using EventInit                                                           //
// 4. Shutdown SteamAPI using EventShutdown                                                        //
// 5. Go to project settings -> Maps and modes -> Gameinstance class -> Select created blueprint  //
///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDE //
////////////
#include "SteamMultiplayer.h"
//////////////////////////
// Constructor - Notes //
////////////////////////////////////////////////////////////////////////////////////////
// 1. No need to initialize Steam callbacks explicitly, STEAM_CALLBACK handles this. //
//////////////////////////////////////////////////////////////////////////////////////
USteamMultiplayer::USteamMultiplayer()
    : bIsHost(false), LobbyIDString("")
{
}
/////////////////////////
// 1. Basic Functions //
///////////////////////////////////////////////
// 1.1 - Initialize SteamAPI                //
// 1.2 - Shutdown SteamAPI                 //
// 1.3 - Check if SteamAPI is initialized //
///////////////////////////////////////////
// - 1.1 - //
void USteamMultiplayer::InitializeSteam()
{
    if (SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Warning, TEXT("Steam is already initialized."));
        return;
    }

    if (SteamAPI_Init())
    {
        UE_LOG(LogTemp, Log, TEXT("Steam API initialized successfully."));

        FString SteamUsername = UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
        uint64 SteamID = SteamUser()->GetSteamID().ConvertToUint64();
        UE_LOG(LogTemp, Log, TEXT("Logged into Steam as: %s (SteamID: %llu)"), *SteamUsername, SteamID);

        if (SteamMatchmaking())
        {
            UE_LOG(LogTemp, Log, TEXT("Steam Matchmaking is available."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Steam Matchmaking is unavailable."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Steam API!"));
    }
}
// - 1.2 - //
void USteamMultiplayer::ShutdownSteam()
{
    SteamAPI_Shutdown();
    UE_LOG(LogTemp, Log, TEXT("Steam API shut down."));
}
// - 1.3 - //
bool USteamMultiplayer::IsSteamInitialized() const
{
    return SteamAPI_IsSteamRunning();
}
//////////////////////
// 2. Hosting Game //
/////////////////////////////////////////////////
// 2.1 - Host a game using Steam Matchmaking  //
// 2.2 - Callback: Lobby created             //
// 2.3 - // Callback: Lobby entered         //
/////////////////////////////////////////////
// - 2.1 - //
void USteamMultiplayer::HostGameWithSteamMatchmaking()
{
    if (!SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API is not initialized."));
        return;
    }

    ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
    if (!SteamMatchmakingTemp)
    {
        UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
        return;
    }

    int32 MaxPlayers = 4;
    int32 LobbyVisibility = k_ELobbyTypePublic;

    SteamAPICall_t CreateLobbyCall = SteamMatchmakingTemp->CreateLobby((ELobbyType)LobbyVisibility, MaxPlayers);
    UE_LOG(LogTemp, Log, TEXT("Lobby creation requested with max players %d"), MaxPlayers);

    FString LobbyMap = "/Game/Maps/Map_Lobby";
    SteamMatchmakingTemp->SetLobbyData(CreateLobbyCall, "MapName", TCHAR_TO_UTF8(*LobbyMap));
}
// - 2.2 - //
void USteamMultiplayer::OnLobbyCreated(LobbyCreated_t* pCallback)
{
    if (pCallback->m_eResult != k_EResultOK)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create lobby: %d"), pCallback->m_eResult);
        return;
    }

    CSteamID LobbyID = CSteamID(pCallback->m_ulSteamIDLobby);
    UE_LOG(LogTemp, Log, TEXT("Lobby created successfully: %llu"), LobbyID.ConvertToUint64());

    // Set up lobby data
    SteamMatchmaking()->SetLobbyData(LobbyID, "GameKey", "urbanshadows");
    SteamMatchmaking()->SetLobbyData(LobbyID, "Region", "Auto");
    SteamMatchmaking()->SetLobbyData(LobbyID, "MapName", "/Game/Maps/Map_Lobby"); // Set the map in lobby data

    // Travel to the map
    FString LobbyMap = SteamMatchmaking()->GetLobbyData(LobbyID, "MapName");

    // Check if the map name is valid
    if (!LobbyMap.IsEmpty())
    {
        // Ensure the map path includes the correct format and append ?listen
        FString TravelCommand = FString::Printf(TEXT("%s?listen"), *LobbyMap);

        // Log the travel command
        UE_LOG(LogTemp, Log, TEXT("Attempting to travel to: %s"), *TravelCommand);

        // Check if the map exists before traveling
        if (FPackageName::DoesPackageExist(LobbyMap))
        {
            // Ensure GetWorld() is valid before calling ServerTravel
            if (GetWorld())
            {
                GetWorld()->ServerTravel(TravelCommand);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("World is not valid, cannot travel."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Map does not exist: %s"), *LobbyMap);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No map name found in lobby data."));
    }
}
// - 2.3 - //
void USteamMultiplayer::OnLobbyEntered(LobbyEnter_t* pCallback)
{
    if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to enter lobby. Response: %d"), pCallback->m_EChatRoomEnterResponse);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully entered lobby: %llu"), pCallback->m_ulSteamIDLobby);

    // Check if the world is valid before traveling
    UWorld* CurrentWorld = GetWorld();
    if (CurrentWorld)
    {
        // Log the map name before traveling
        FString MapName = "/Game/Maps/Map_Lobby"; // This should match the actual map you're trying to travel to
        UE_LOG(LogTemp, Log, TEXT("Attempting to travel to map: %s"), *MapName);

        // Check if the map exists
        if (FPackageName::DoesPackageExist(MapName))
        {
            CurrentWorld->ServerTravel(MapName);
            UE_LOG(LogTemp, Log, TEXT("ServerTravel to %s was successful."), *MapName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Map %s does not exist or is not packaged."), *MapName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World is not valid, cannot travel."));
    }
}
///////////////////////
// 3. Finding Games //
////////////////////////////////////////////////////////////////////////
// 3.1 - Find lobbies with specified settings, example: region, tag  //
// 3.2 - Callback: Found lobbies                                    //
// 3.3 - Join lobby by using LobbyID                               //
////////////////////////////////////////////////////////////////////
// - 3.1 - //
void USteamMultiplayer::FindLobbiesWithSettings(FString Tag, FString Region)
{
    if (!SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API is not initialized."));
        return;
    }

    ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
    if (!SteamMatchmakingTemp)
    {
        UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
        return;
    }

    // Reset the lobby array before searching
    FoundLobbies.Empty();

    // Add search filters for the tag and region
    SteamMatchmakingTemp->AddRequestLobbyListStringFilter("GameKey", TCHAR_TO_UTF8(*Tag), k_ELobbyComparisonEqual);
    //SteamMatchmakingTemp->AddRequestLobbyListStringFilter("Region", TCHAR_TO_UTF8(*Region), k_ELobbyComparisonEqual);

    // Request the list of lobbies
    SteamAPICall_t SearchCall = SteamMatchmakingTemp->RequestLobbyList();
    UE_LOG(LogTemp, Log, TEXT("Lobby search requested for tag: %s, region: %s"), *Tag, *Region);
}
// - 3.2 - //
void USteamMultiplayer::OnLobbyListReceived(LobbyMatchList_t* pCallback)
{
    if (pCallback->m_nLobbiesMatching == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("No matching lobbies found."));
        return;
    }

    // Loop through the found lobbies and store their info
    for (uint32 i = 0; i < pCallback->m_nLobbiesMatching; ++i)
    {
        CSteamID LobbyID = SteamMatchmaking()->GetLobbyByIndex(i);
        FString LobbyIDStringTemp = FString::Printf(TEXT("%llu"), LobbyID.ConvertToUint64());
        FString HostName = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "name"));
        int32 CurrentPlayers = SteamMatchmaking()->GetNumLobbyMembers(LobbyID);
        FString MaxPlayersStr = SteamMatchmaking()->GetLobbyData(LobbyID, "maxplayers");
        int32 MaxPlayers = FCString::Atoi(*MaxPlayersStr);
        FString Region = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "Region"));
        FString MapName = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "MapName"));

        // Create a lobby struct and add it to the array
        FLobbyInfoData LobbyInfo;
        LobbyInfo.HostName = HostName;
        LobbyInfo.CurrentPlayers = CurrentPlayers;
        LobbyInfo.MaxPlayers = MaxPlayers;
        LobbyInfo.Region = Region;
        LobbyInfo.MapName = MapName;
        LobbyInfo.LobbyIDString = LobbyIDStringTemp;

        FoundLobbies.Add(LobbyInfo);
    }

    // Notify UI about the found lobbies (optional)
    //OnLobbiesFound.Broadcast();
}
// - 3.3 - //
void USteamMultiplayer::JoinLobby(FString LobbyID)
{
    if (!SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API is not initialized."));
        return;
    }

    ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
    if (!SteamMatchmakingTemp)
    {
        UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
        return;
    }

    FString LobbyIDStringTemp = LobbyID;
    uint64 LobbyIDNumeric = FCString::Strtoui64(*LobbyIDStringTemp, nullptr, 10);
    CSteamID LobbyIDSteamFormat((uint64)LobbyIDNumeric);
    SteamMatchmaking()->JoinLobby(LobbyIDSteamFormat);
    UE_LOG(LogTemp, Log, TEXT("Attempting to join lobby: %llu"), LobbyIDSteamFormat.ConvertToUint64());
}