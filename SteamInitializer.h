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

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "steam/steam_api.h"
#include "SteamInitializer.generated.h"

//Details of the sessions found
USTRUCT(BlueprintType)
struct FCustomBlueprintSessionResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "SteamMultiplayer")
    FString OwnerName;

    UPROPERTY(BlueprintReadOnly, Category = "SteamMultiplayer")
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "SteamMultiplayer")
    int32 CurrentPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "SteamMultiplayer")
    FString SessionName;
};
//Details of the players in lobby 
USTRUCT(BlueprintType)
struct FPlayerLobbyInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    UTexture2D* PlayerAvatar;  // Make sure to set this texture in the game

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bIsReady;
};

UCLASS()
class URBANSHADOWS_API USteamInitializer : public UGameInstance
{
    GENERATED_BODY()


    ////////////////
    // PROTECTED //
    //////////////////////////
    // 1. Initialize Steam //
    // 2. Shutdown Steam  //
    ///////////////////////
protected:
    // -1- //
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InitializeSteam();

    // -2- //
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void ShutdownSteam();

    //////////////
    // PUBLIC  //
    ///////////////////////////////////////////////////////////////////////////////
    // 1. Check if steam is initialized                                         //
    // 2. Host/Create Game                                                     //
    // 3. Find Games                                                          //
    // 4. Return found games to blueprints                                   //
    // 5. Join Game                                                         //
    // 6. Register player to the session                                   //
    // 7. Check if all players are ready                                  //
    // 8. Start the game (only callable by the host)                     //
    // 9. Update player's ready status                                  //
    // 10. Update a player's information (name, avatar, ready status)  //
    // 11. Is the local player the host?                              //
    // 12. GameSessionID                                             //
    // 13. RandomSessionName                                        //
    // 14. Array of players in lobby - WIP                         //
    // 15. Map to track players in the lobby - WIP                //
    // 16. Synchronize player list across clients - WIP          //
    //////////////////////////////////////////////////////////////
public:
    // -1- //
    UFUNCTION(BlueprintCallable, Category = "Steam")
    bool IsSteamInitialized() const;

    // -2- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void HostServer(int32 MaxPlayers, FString TargetMap);

    // -3- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void FindGames(int32 NumberOfGames, FString InGameSessionID, bool UsingGameSessionID);

    // -4- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    TArray<FCustomBlueprintSessionResult> GetFoundSessions() const;

    // -5- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void JoinGame(int32 SessionIndex, FName SessionName);

    // -6- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void RegisterPlayer(FName SessionName);

    // -7- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    bool AreAllPlayersReady() const;

    // -8- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void StartGame();

    // -9- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void SetPlayerReadyStatus(const FString& PlayerName, bool bIsReady);

    // -10- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void UpdatePlayerInfo(const FString& PlayerName, UTexture2D* PlayerAvatar, bool bIsReady);

    // -11- //
    UPROPERTY(BlueprintReadWrite, Category = "SteamMultiplayer")
    bool bIsHost;

    // -12- //
    UPROPERTY(BlueprintReadWrite, Category = "SteamMultiplayer")
    FString GameSessionID;

    // -13- //
    UPROPERTY(BlueprintReadWrite, Category = "SteamMultiplayer")
    FString RandomName;

    // -14- //
    UPROPERTY(BlueprintReadWrite, Category = "SteamMultiplayer")
    TArray<FPlayerLobbyInfo> PlayerLobbyInfoArray;

    // -15- //
    UPROPERTY(BlueprintReadWrite, Category = "SteamMultiplayer")
    TMap<FString, FPlayerLobbyInfo> PlayerLobbyInfoMap;

    // -16- //
    UFUNCTION(BlueprintCallable, Category = "SteamMultiplayer")
    void BroadcastLobbyUpdate();


    //////////////
    // PRIVATE //
    ///////////////////////////////////////////////////////
    // 1. List of found sessions                        //
    // 2. Current session search settings              //
    // 3. Callback when session search completes      //
    // 4. Callback for join session completion       //
    // 5. Callback for session creation completion  //
    /////////////////////////////////////////////////
private:
    // -1- //
    TArray<FOnlineSessionSearchResult> FoundSessions;

    // -2- //
    TSharedPtr<class FOnlineSessionSearch> CurrentSearchSettings;

    // -3- //
    void OnFindSessionsComplete(bool bSuccess);

    // -4- //
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    // -5- //
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
};
///////////////////////////////////////////
// MORE COMING SOON  // A.S. - hajddeen //
/////////////////////////////////////////
