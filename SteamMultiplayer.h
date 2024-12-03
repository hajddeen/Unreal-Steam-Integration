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
#pragma once
/////////////////////////////////////////////////////////
// INCLUDE //
////////////
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "steam/steam_api.h"
#include "steam/isteammatchmaking.h"
#include "SteamMultiplayer.generated.h"

///////////////////////////////////////
// STRUCT TO HOLD LOBBY INFORMATION //
///////////////////////////////////////
USTRUCT(BlueprintType)
struct FLobbyInfoData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    FString HostName;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    int32 CurrentPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    FString Region;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    FString MapName;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby Info")
    FString LobbyIDString; // Use FString instead of CSteamID
};
///////////////////////////////////////
// STRUCT TO HOLD PLAYER INFORMATION //
///////////////////////////////////////
USTRUCT(BlueprintType)
struct FLobbyPlayerInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Player Info")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Player Info")
    UTexture2D* PlayerAvatar;
};
////////////////
// MAIN BODY //
////////////////
UCLASS()
class URBANSHADOWS_API USteamMultiplayer : public UGameInstance
{
    GENERATED_BODY()

public:
    //////////////////////////////////////////////
    // 1. Constructor and Initialization       //
    ////////////////////////////////////////////
    USteamMultiplayer();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InitializeSteam();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void ShutdownSteam();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    bool IsSteamInitialized() const;

    //////////////////////////////////////////////
    // 2. Steam Matchmaking and Multiplayer    //
    ////////////////////////////////////////////
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void HostGameWithSteamMatchmaking();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void FindLobbiesWithSettings(FString Tag, FString Region);

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void JoinLobby(FString LobbyID);


    UFUNCTION(BlueprintCallable, Category = "Steam")
    TArray<FLobbyPlayerInfo> GetLobbyMembersWithAvatars();

protected:
    //////////////////////////////////////////////
    // 1. Steam Callbacks                      //
    ////////////////////////////////////////////    
    STEAM_CALLBACK(USteamMultiplayer, OnLobbyCreated, LobbyCreated_t);
    STEAM_CALLBACK(USteamMultiplayer, OnLobbyEntered, LobbyEnter_t);
    STEAM_CALLBACK(USteamMultiplayer, OnLobbyListReceived, LobbyMatchList_t);

    //////////////////////////////////////////////
    // 2. Blueprint-Accessible Members         //
    ////////////////////////////////////////////
    UPROPERTY(BlueprintReadOnly, Category = "Lobbies")
    TArray<FLobbyInfoData> FoundLobbies;

private:
    //////////////////////////////////////////////
    // 1. Private Data Members                 //
    ////////////////////////////////////////////
    bool bIsHost;          // Is the player hosting the game?
    FString LobbyIDString; // Current Lobby ID as string
    UTexture2D* GetAvatarTexture(int AvatarHandle);
};
///////////////////////////////////////////
// MORE COMING SOON  // A.S. - hajddeen //
/////////////////////////////////////////
