#pragma once
#include <winsock2.h>
#include <Windows.h>
#include "Network.h"
#include <ws2tcpip.h>
#include <stdio.h> 
#include <string>

#define DEFAULT_PORT "8000"

#pragma comment (lib, "Ws2_32.lib")

namespace Client
{
    class ClientNetwork
    {
    private:
        int iResult;
        SOCKET connectSocket;
        WSAEVENT sockEvent;
        WSANETWORKEVENTS networkEvents;
        bool connected = false;

    public:
        ClientNetwork(std::string& adress);
        ~ClientNetwork();

        int receivePackets(char* recvbuf);
        SOCKET& getSocket();
        WSAEVENT& getSockEvent();
        WSANETWORKEVENTS& getNetworkEvents();
        void extractEvents();
        bool& isConnected();
    };
}