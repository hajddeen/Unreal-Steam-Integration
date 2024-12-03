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
//////////////////////////////////////////////////////////////////
// LICENSE AGREEMENT // LICENSE AGREEMENT // LICENSE AGREEMENT //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                                                                                                                                          //
// Last Updated: 03/12/2024                                                                                                                                                                                                                                //
//                                                                                                                                                                                                                                                        //
// 1. Grant of License                                                                                                                                                                                                                                   //
// This license permits anyone to use, modify, and distribute the provided code (the "Code") in projects free of charge under the following conditions:                                                                                                 //
// 1.1. The Code must only be used for lawful purposes.                                                                                                                                                                                                //
// 1.2. The user must include proper credit in their project as described in Section 3 below.                                                                                                                                                         //
//                                                                                                                                                                                                                                                   //
// 2. Restrictions                                                                                                                                                                                                                                  //
// 2.1. This Code cannot be sold, sublicensed, or redistributed as a standalone product or as part of a similar development toolkit.                                                                                                               //
// 2.2. The Code must not be used for projects that violate local laws or promote harmful, discriminatory, or illegal activities.                                                                                                                 //
// 2.3. The user must not remove, alter, or obscure this license agreement, attribution requirements, or copyright notices within the Code.                                                                                                      //
//                                                                                                                                                                                                                                              //
// 3. Attribution                                                                                                                                                                                                                              //
// 3.1. The user must include the following credit in a visible location within their project (e.g., splash screen, credits screen, or documentation):                                                                                        //
// "Steam Multiplayer Code developed by Adrian Szajewski. Used under license."                                                                                                                                                               //
// 3.2. For distributed or published projects, a link to https://adrianszajewskidev.wordpress.com/ or other contact details must also be included if applicable.                                                                            //
//                                                                                                                                                                                                                                         //
// 4. Breach of Agreement                                                                                                                                                                                                                 //
// 4.1. If this license is violated, the licensor reserves the right to revoke the user's rights to use the Code immediately.                                                                                                            //
// 4.2. The licensor may take legal action to recover damages or enforce compliance, depending on the severity of the violation.                                                                                                        //
// 4.3. Users found in breach of this license may be required to:                                                                                                                                                                      //
// Remove the Code from their project(s).                                                                                                                                                                                             //
// Cease distribution of any projects using the Code.                                                                                                                                                                                //
//                                                                                                                                                                                                                                  //
// 5. Disclaimer                                                                                                                                                                                                                   //
// 5.1. The Code is provided "as is" without any warranty, express or implied. The licensor is not liable for any damages arising from the use or misuse of the Code.                                                             //
// 5.2. It is the user's responsibility to ensure compatibility and safe usage of the Code in their projects.                                                                                                                    //
//                                                                                                                                                                                                                              //
// 6. Governing Law                                                                                                                                                                                                            //
// This agreement shall be governed and construed in accordance with the laws of Poland. Any disputes arising under or in connection with this license will be subject to the exclusive jurisdiction of the courts of Poland. //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    else
    {
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
    else
    {
        ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
        if (!SteamMatchmakingTemp)
        {
            UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
            return;
        }
        else
        {
            int32 MaxPlayers = 4;
            int32 LobbyVisibility = k_ELobbyTypePublic;

            SteamAPICall_t CreateLobbyCall = SteamMatchmakingTemp->CreateLobby((ELobbyType)LobbyVisibility, MaxPlayers);
            UE_LOG(LogTemp, Log, TEXT("Lobby creation requested with max players %d"), MaxPlayers);

            FString LobbyMap = "/Game/Maps/Map_Lobby";
            SteamMatchmakingTemp->SetLobbyData(CreateLobbyCall, "MapName", TCHAR_TO_UTF8(*LobbyMap));
        }
    }
}
// - 2.2 - //
void USteamMultiplayer::OnLobbyCreated(LobbyCreated_t* pCallback)
{
    if (pCallback->m_eResult != k_EResultOK)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create lobby: %d"), pCallback->m_eResult);
        return;
    }
    else
    {
        CSteamID LobbyID = CSteamID(pCallback->m_ulSteamIDLobby);
        LobbyIDString = FString::Printf(TEXT("%llu"), LobbyID.ConvertToUint64());
        UE_LOG(LogTemp, Log, TEXT("Lobby created successfully: %llu"), LobbyID.ConvertToUint64());

        // Set up lobby data
        CSteamID HostSteamID = SteamMatchmaking()->GetLobbyOwner(LobbyID);
        const char* HostName = SteamFriends()->GetFriendPersonaName(HostSteamID);
        SteamMatchmaking()->SetLobbyData(LobbyID, "HostName", HostName);
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
}
// - 2.3 - //
void USteamMultiplayer::OnLobbyEntered(LobbyEnter_t* pCallback)
{
    if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to enter lobby. Response: %d"), pCallback->m_EChatRoomEnterResponse);
        return;
    }
    else
    {
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
    else
    {
        ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
        if (!SteamMatchmakingTemp)
        {
            UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
            return;
        }
        else
        {
            // Reset the lobby array before searching
            FoundLobbies.Empty();

            // Add search filters for the tag and region
            SteamMatchmakingTemp->AddRequestLobbyListStringFilter("GameKey", TCHAR_TO_UTF8(*Tag), k_ELobbyComparisonEqual);
            //SteamMatchmakingTemp->AddRequestLobbyListStringFilter("Region", TCHAR_TO_UTF8(*Region), k_ELobbyComparisonEqual);

            // Request the list of lobbies
            SteamAPICall_t SearchCall = SteamMatchmakingTemp->RequestLobbyList();
            UE_LOG(LogTemp, Log, TEXT("Lobby search requested for tag: %s, region: %s"), *Tag, *Region);
        }
    }
}
// - 3.2 - //
void USteamMultiplayer::OnLobbyListReceived(LobbyMatchList_t* pCallback)
{
    if (pCallback->m_nLobbiesMatching == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("No matching lobbies found."));
        return;
    }
    else
    {
        // Loop through the found lobbies and store their info
        for (uint32 i = 0; i < pCallback->m_nLobbiesMatching; ++i)
        {
            CSteamID LobbyID = SteamMatchmaking()->GetLobbyByIndex(i);
            FString LobbyIDStringTemp = FString::Printf(TEXT("%llu"), LobbyID.ConvertToUint64());
            FString HostNameString = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "HostName"));
            int32 CurrentPlayers = SteamMatchmaking()->GetNumLobbyMembers(LobbyID);
            int32 MaxPlayers = SteamMatchmaking()->GetLobbyMemberLimit(LobbyID);
            FString Region = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "Region"));
            FString MapName = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(LobbyID, "MapName"));

            // Create a lobby struct and add it to the array
            FLobbyInfoData LobbyInfo;
            LobbyInfo.HostName = HostNameString;
            LobbyInfo.CurrentPlayers = CurrentPlayers;
            LobbyInfo.MaxPlayers = MaxPlayers;
            LobbyInfo.Region = Region;
            LobbyInfo.MapName = MapName;
            LobbyInfo.LobbyIDString = LobbyIDStringTemp;

            FoundLobbies.Add(LobbyInfo);
        }
    }
}
// - 3.3 - //
void USteamMultiplayer::JoinLobby(FString LobbyID)
{
    if (!SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Error, TEXT("Steam API is not initialized."));
        return;
    }
    else
    {
        ISteamMatchmaking* SteamMatchmakingTemp = SteamMatchmaking();
        if (!SteamMatchmakingTemp)
        {
            UE_LOG(LogTemp, Error, TEXT("Steam Matchmaking interface is not available!"));
            return;
        }
        else
        {
            FString LobbyIDStringTemp = LobbyID;
            uint64 LobbyIDNumeric = FCString::Strtoui64(*LobbyIDStringTemp, nullptr, 10);
            CSteamID LobbyIDSteamFormat((uint64)LobbyIDNumeric);
            LobbyIDString = LobbyID;
            bIsHost = false;
            SteamMatchmaking()->JoinLobby(LobbyIDSteamFormat);
            UE_LOG(LogTemp, Log, TEXT("Attempting to join lobby: %llu"), LobbyIDSteamFormat.ConvertToUint64());
        }
    }
}
///////////////////////
// 4. Lobby Data    //
/////////////////////////////////////////////////////
// 4.1 - Get lobby members informations + avatar  //
// 4.2 - Get avatar texture                      //
//////////////////////////////////////////////////
// - 4.1 - //
TArray<FLobbyPlayerInfo> USteamMultiplayer::GetLobbyMembersWithAvatars()
{
    TArray<FLobbyPlayerInfo> PlayerInfos;

    FString LobbyIDStringTemp = LobbyIDString;
    uint64 LobbyIDNumeric = FCString::Strtoui64(*LobbyIDStringTemp, nullptr, 10);
    CSteamID LobbyIDSteamFormat((uint64)LobbyIDNumeric);

    int32 NumMembers = SteamMatchmaking()->GetNumLobbyMembers(LobbyIDSteamFormat);
    for (int32 i = 0; i < NumMembers; ++i)
    {
        CSteamID MemberID = SteamMatchmaking()->GetLobbyMemberByIndex(LobbyIDSteamFormat, i);
        FLobbyPlayerInfo PlayerInfo;

        const char* Name = SteamFriends()->GetFriendPersonaName(MemberID);
        PlayerInfo.PlayerName = Name ? FString(Name) : TEXT("Unknown");

        int AvatarHandle = SteamFriends()->GetLargeFriendAvatar(MemberID);
        PlayerInfo.PlayerAvatar = AvatarHandle != -1 ? GetAvatarTexture(AvatarHandle) : nullptr;

        PlayerInfos.Add(PlayerInfo);
    }

    return PlayerInfos;
}
// - 4.2 - //
UTexture2D* USteamMultiplayer::GetAvatarTexture(int AvatarHandle)
{
    // Get the image size from Steam API
    uint32 Width = 0, Height = 0;
    if (!SteamUtils()->GetImageSize(AvatarHandle, &Width, &Height) || Width == 0 || Height == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get avatar image size."));
        return nullptr;
    }
    else
    {
        // Retrieve the raw RGBA data from Steam
        TArray<uint8> AvatarRGBA;
        AvatarRGBA.AddUninitialized(Width * Height * 4);
        if (!SteamUtils()->GetImageRGBA(AvatarHandle, AvatarRGBA.GetData(), AvatarRGBA.Num()))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get avatar image RGBA data."));
            return nullptr;
        }
        else
        {
            // Create a new transient texture
            UTexture2D* AvatarTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
            if (!AvatarTexture)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create transient texture."));
                return nullptr;
            }
            else
            {
                // Lock the texture for editing
                void* TextureData = AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                if (!TextureData)
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to lock texture data for editing."));
                    return nullptr;
                }
                else
                {
                    // Copy the RGBA data into the texture
                    FMemory::Memcpy(TextureData, AvatarRGBA.GetData(), AvatarRGBA.Num());
                    AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

                    // Update the texture
                    AvatarTexture->UpdateResource();

                    // Return the created texture
                    return AvatarTexture;
                }
            }
        }
    }
}
///////////////////////////////////////////
// MORE COMING SOON  // A.S. - hajddeen //
/////////////////////////////////////////
