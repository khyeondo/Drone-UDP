// Fill out your copyright notice in the Description page of Project Settings.


#include <thread> 

#include "NetworkingLevelActor.h"

ANetworkingLevelActor::~ANetworkingLevelActor()
{
    if (isConnected) {
        closesocket(io);
        WSACleanup();
    }

    if (reciver != nullptr)
        delete reciver;            
}

bool ANetworkingLevelActor::InitNetwork()
{
    version = MAKEWORD(2, 2);
    wsOK = WSAStartup(version, &data); //Windows ���� �ʱ�ȭ

    if (wsOK != 0) {
        UE_LOG(LogTemp, Error, TEXT("Can't start Winsock  %d"), wsOK);
        return false;
    }

    io = socket(AF_INET, SOCK_DGRAM, 0); //���� ����

    return true;
}

bool ANetworkingLevelActor::StartReceive(int port)
{
    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(port);

    // ������ ������ ��� 
    if (bind(io, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        UE_LOG(LogTemp, Error, TEXT("Can't bind socket! %d"), WSAGetLastError());
        return false;
    }

    clientLenth = sizeof(client);
    ZeroMemory(&client, clientLenth);

    StartReceiveThread(); //Receive ������ ���� 

    return true;
}

bool ANetworkingLevelActor::SetSendInfo(FString ip, int port)
{
    server.sin_family = AF_INET;
    server.sin_port = htons(port);   
    inet_pton(AF_INET, TCHAR_TO_ANSI(*ip), &server.sin_addr);

    return true;
}



void ANetworkingLevelActor::StartReceiveThread()
{
    if (reciver == nullptr) {
        //Recv()�Լ��� ������ ����
        reciver = new FReciver([this]() { this->Recv(); });
    }
}

void ANetworkingLevelActor::Send(FVector p, FRotator r)
{
    float f[BUFFERSIZE/4];    

    f[0] = p.X;
    f[1] = p.Y;
    f[2] = p.Z;
    f[3] = r.Pitch;
    f[4] = r.Yaw;
    f[5] = r.Roll;

    int sendOk = sendto(io, (const char*)&f, BUFFERSIZE, 0, (sockaddr*)&server, sizeof(server));

    if (sendOk == SOCKET_ERROR) {
        UE_LOG(LogTemp, Warning, TEXT("Can't start Winsock: %d"), WSAGetLastError);
    }
}

FVector ANetworkingLevelActor::SyncedPos()
{
    return FVector(*(float*)buf, *(float*)(buf + 4), *(float*)(buf + 8));
}

FRotator ANetworkingLevelActor::SyncedRot()
{
    return FRotator(*(float*)(buf + 12), *(float*)(buf + 16), *(float*)(buf + 20));
}

bool ANetworkingLevelActor::IsConnected()
{
    return isConnected;
}

void ANetworkingLevelActor::Recv()
{
    //buf������ �޾ƿ� ��ġ,ȸ������ ����
    int bytesIn = recvfrom(io, buf, BUFFERSIZE, 0, (sockaddr*)&client, &clientLenth); 
    
    if (bytesIn == SOCKET_ERROR) {
        UE_LOG(LogTemp, Error, TEXT("Can't start Winsock: %d"), WSAGetLastError);
        return;
    }    
    char clientIp[256];
    ZeroMemory(clientIp, 256);
    
    inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
}

FReciver::FReciver(std::function<void()> func)
{
    this->func = func;
    Thread = FRunnableThread::Create(this, TEXT("UDP Reciver"));
}

FReciver::~FReciver()
{
    if(Thread) 
    {
        Thread->Kill();
        delete Thread;
    }
}

bool FReciver::Init()
{
    return true;
}

uint32 FReciver::Run()
{
    while (bRunThread) {
        func();
    }
    return 0;
}

void FReciver::Stop()
{
    bRunThread = false;
}
