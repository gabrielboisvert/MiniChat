#pragma once
#include <winsock2.h>
#include <Windows.h>
#include "Network.h"
#include <ws2tcpip.h>
#include <map>
#include <string>
#include <vector>
#pragma comment (lib, "Ws2_32.lib")

namespace Server
{
    class ServerNetwork
    {
    private:
        SOCKET listenSocket;
        SOCKET clientSocket;
        int iResult;
        std::map<SOCKET, unsigned int> sessions;
        std::map<SOCKET, std::string> pseudos;
        char network_data[MAX_PACKET_SIZE] = { 0 };
        static unsigned int client_id;

    public:
        ServerNetwork();
        ~ServerNetwork();
        bool acceptNewClient(unsigned int id);
        void receiveFromClient(SOCKET& socket);
        void update();

        void sendConnectedUser(std::string& str, SOCKET& socket);

        int receiveData(SOCKET& socket, char* recvbuf);

        void sendToAll(char* packets, int totalSize);
        void sendToAllExept(char* packets, int totalSize, SOCKET& client);
        void sendToOne(char* packets, int totalSize, SOCKET& client);

        std::map<SOCKET, unsigned int>& getSessions();
        std::map<SOCKET, std::string>& getPseudos();
        std::vector<pollfd> polls;

        void getIpV4Str();
        void getIpV6Str();
    };
}