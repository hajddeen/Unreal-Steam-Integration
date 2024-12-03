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
};
