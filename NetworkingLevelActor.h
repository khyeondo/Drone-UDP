// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <WS2tcpip.h>
#include <functional>
#pragma comment(lib, "ws2_32.lib")

#include "HAL/Runnable.h"

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "NetworkingLevelActor.generated.h"


#define BUFFERSIZE 24

/**
 * 
 */
class FReciver;

UCLASS()
class BLOCKS_API ANetworkingLevelActor : public ALevelScriptActor
{
    GENERATED_BODY()

private:
    WSADATA data;
    WORD version;
    int wsOK;
    SOCKET io;
    sockaddr_in server;
    sockaddr_in client;
    int clientLenth;
    bool isConnected = false;

    FReciver* reciver = nullptr;

    char buf[BUFFERSIZE];

public:
    ~ANetworkingLevelActor();

    UFUNCTION(BlueprintCallable)
    bool InitNetwork();

    UFUNCTION(BlueprintCallable)
    bool StartReceive(int port);
	
    UFUNCTION(BlueprintCallable)
    bool SetSendInfo(FString ip, int port);

    UFUNCTION(BlueprintCallable)
    void Send(FVector v, FRotator r);

    UFUNCTION(BlueprintPure)
    FVector SyncedPos();
    UFUNCTION(BlueprintPure)
    FRotator SyncedRot();
    
    UFUNCTION(BlueprintPure)
    bool IsConnected();

private:
    void StartReceiveThread();
    void Recv();
};

class BLOCKS_API FReciver: public FRunnable
{
public:
    FReciver(std::function<void()> func);

    virtual ~FReciver() override;

    bool Init() override;
    uint32 Run() override;
    void Stop() override;

private:
    FRunnableThread* Thread;
    bool bRunThread;

    std::function<void()> func;
};