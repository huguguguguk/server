#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#pragma comment( lib, "ws2_32.lib")

unsigned int __stdcall ThreadFunc(void* _pArgs)
{
    //char sendData[255];
    int recvByte;
    char recvData[255];
    int recvCount = 0;
    SOCKET socket = *(SOCKET*)_pArgs;

    while ((recvByte = recv(socket, recvData, sizeof(recvData), 0)) > 0)
    {
        //받은내용 보내기.
        recvData[recvByte] = '\0';
        printf("recv 채팅 : %s \n", recvData);
    }

    printf("close thread \n");
    return 0;
}


int main(void)
{
    int servPort = 30002;
    int i = 0;
    unsigned int threadID;

    WSADATA wsaData;
    SOCKET sockServer;

    // 1. 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Failed WSAStartup() \n");
        return 1;
    }

    // 2. 소켓 생성
    sockServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockServer == INVALID_SOCKET)
    {
        printf("Failed socket() \n");
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN addrServer;
    // 3. 서버 연결 설정
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    //addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr);
    addrServer.sin_port = htons(servPort);
    if (connect(sockServer, (SOCKADDR*)&addrServer, sizeof(addrServer)) == SOCKET_ERROR)
    {
        printf("Listen Error \n");
        closesocket(sockServer);
        WSACleanup();
        return 1;
    }

    printf("Connect .... \n");

    HANDLE hThread;

    hThread = (HANDLE)_beginthreadex(NULL, 0, &ThreadFunc, (void*)&sockServer, 0, &threadID);
    printf("start recv thread \n");

    char chat[255];
    int chatSize = 0;
    while (1)
    {
        scanf_s("%s", chat, (unsigned)sizeof(chat));
        if (strcmp(chat, "exit") == 0) break;
        chatSize = strnlen_s(chat, sizeof(chat));
        if (chatSize > 0) send(sockServer, chat, chatSize, 0);
    }

    CloseHandle(hThread);

    // 5. 소켓 종료 -> 윈속 종료
    closesocket(sockServer);
    WSACleanup();

    return 0;
}