#include "pch.h"
#include "ClientNetwork.h"
#include "Network.h"
#include <string>

using namespace Client;
using namespace Network;

ClientNetwork::ClientNetwork(std::string& adress)
{
    WSADATA wsaData;
    this->connectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL, *ptr = NULL, hints;

    this->iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (this->iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    iResult = getaddrinfo(adress.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }


    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
    {
        this->connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (this->connectSocket == INVALID_SOCKET) 
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return;
        }

        iResult = connect(this->connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(this->connectSocket);
            this->connectSocket = INVALID_SOCKET;
            printf("The server is down... did not connect\n");
            return;
        }
    }


    freeaddrinfo(result);


    if (this->connectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return;
    }
    
    u_long iMode = 1;
    iResult = ioctlsocket(this->connectSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(this->connectSocket);
        WSACleanup();
        this->connectSocket = INVALID_SOCKET;
        return;
    }

    char value = 1;
    setsockopt(this->connectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));


    this->sockEvent = WSACreateEvent();
    WSAEventSelect(connectSocket, sockEvent, FD_READ | FD_CLOSE);

    this->connected = true;
}

ClientNetwork::~ClientNetwork()
{
    closesocket(this->connectSocket);
    WSACleanup();
}

int ClientNetwork::receivePackets(char* recvbuf)
{
    iResult = NetworkServices::receiveMessage(this->connectSocket, recvbuf, MAX_PACKET_SIZE);

    if (iResult == 0)
    {
        printf("Connection closed\n");
        closesocket(this->connectSocket);
        WSACleanup();
        return -1;
    }

    return iResult;
}

SOCKET& Client::ClientNetwork::getSocket()
{
    return this->connectSocket;
}

WSAEVENT& Client::ClientNetwork::getSockEvent()
{
    return this->sockEvent;
}

WSANETWORKEVENTS& Client::ClientNetwork::getNetworkEvents()
{
    return this->networkEvents;
}

void Client::ClientNetwork::extractEvents()
{
    WSAEnumNetworkEvents(connectSocket, sockEvent, &networkEvents);
    WSAResetEvent(sockEvent);
}

bool& Client::ClientNetwork::isConnected()
{
    return this->connected;
}
