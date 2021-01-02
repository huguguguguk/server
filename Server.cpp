#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#pragma comment( lib, "ws2_32.lib")
/*
unsigned int __stdcall ThreadFunc(void* _pArgs)
{
    //char sendData[255];
    SOCKET servSocket = *(SOCKET*)_pArgs;
    SOCKET clientSocket;
    SOCKADDR_IN clientAddr;

    int sizeClientAddr = sizeof(clientAddr);
    char sendData[255] = "HI";
    char recvByte;
    char recvData[255];
    int recvCount = 0;


    while ((recvByte = recv(clientSocket, recvData, sizeof(recvData), 0)) > 0)
    {
        // 4. 접속이 시도된 클라이언트 연결과 통신 처리
        // 4-2. 연결된 클라이언트에게 send
        send(clientSocket, sendData, strlen(sendData) + 1, 0);

    }

    // 클라이언트 연결 종료
    printf("close socket : %d \n", clientSocket);
    closesocket(clientSocket);
    printf("close thread \n");
    return 0;
}
*/
int main(void)
{
    WSADATA wsaData;
    SOCKET listenSocket;
    SOCKADDR_IN servAddr;

    // 1. 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Failed WSAStartup() \n");
        return 1;
    }

    // 2. 소켓 생성
    listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        WSACleanup();
        printf("Failed socket() \n");
        return 1;
    }

    // 3. 서버 연결
    // 3-1. 서버 주소 세팅
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    //servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
    servAddr.sin_port = htons(30002);

    if (bind(listenSocket,(SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        printf("Binding Error \n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        printf("Listen Error \n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    printf("Server Start \n");

    // 4. 접속이 시도된 클라이언트 연결과 통신 처리
    // select 함수 세팅
    fd_set fdSocketInfors;
    fd_set fdReads;
    int iRet;
    unsigned int i;
    SOCKET sockClient, sockTemp;
    SOCKADDR_IN addrClient;
    int addrSize;
    char recvBuffer[255];
    int recvSize;

    unsigned int threadID;
    // HANDLE hThread;

    FD_ZERO(&fdSocketInfors);
    FD_SET(listenSocket, &fdSocketInfors);
    while (1)
    {
        fdReads = fdSocketInfors;
        iRet = select(0, &fdReads, 0, 0, 0);
        if (iRet == SOCKET_ERROR) break;

        for (i = 0; i < fdSocketInfors.fd_count; i++)
        {
            sockTemp = fdSocketInfors.fd_array[i];
            if (FD_ISSET(sockTemp, &fdReads))
            {
                if (sockTemp == listenSocket)
                {
                    addrSize = sizeof(addrClient);
                    sockClient = accept(listenSocket, (SOCKADDR*)&addrClient, &addrSize);
                    if (sockClient == INVALID_SOCKET) printf("Failed accept() \n");
                    FD_SET(sockClient, &fdSocketInfors);
                    printf("%d connected %d %d,%d,%d,%d : %d \n", i, sockClient,
                        addrClient.sin_addr.S_un.S_un_b.s_b1,
                        addrClient.sin_addr.S_un.S_un_b.s_b2,
                        addrClient.sin_addr.S_un.S_un_b.s_b3,
                        addrClient.sin_addr.S_un.S_un_b.s_b4,
                        addrClient.sin_port);
                    // hThread = (HANDLE)_beginthreadex(NULL, 0, &ThreadFunc, (void*)&sockTemp, 0, &threadID);
                    // printf("start recv thread \n");
                    // WaitForSingleObject(hThread, INFINITE);
                    // CloseHandle(hThread);
                }
                else // client인 경우 쓰레드생성(클라이언트소켓 생성
                {
                    recvSize = recv(sockTemp, recvBuffer, sizeof(recvBuffer), 0);
                    printf("%d : socket = %d recv \n", i, sockTemp);
                    if (recvSize > 0) send(sockTemp, recvBuffer, recvSize, 0);
                    else
                    {
                        FD_CLR(sockTemp, &fdSocketInfors);
                        closesocket(sockTemp);
                        printf("close socket : %d \n", sockTemp);
                    }
                }
            }
        }

    }


    // 5. 소켓 종료 -> 윈속 종료
    closesocket(listenSocket);
    WSACleanup();

    system("pause");
    return 0;
}