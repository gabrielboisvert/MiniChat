#include "pch.h"
#include "ServerNetwork.h"
#include "Network.h"
#include <string>


using namespace Server;
using namespace Network;

unsigned int ServerNetwork::client_id;

ServerNetwork::ServerNetwork()
{
    client_id = 0;

    WSADATA wsaData;

    this->listenSocket = INVALID_SOCKET;
    this->clientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(1);
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) 
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(1);
    }


    this->listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (this->listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    DWORD ipv6only = 0;
    iResult = setsockopt(this->listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only));
    if (iResult == SOCKET_ERROR)
    {
        printf("Failed to set option: %d\n", WSAGetLastError());
        closesocket(this->listenSocket);
        WSACleanup();
        exit(1);
    }


    u_long iMode = 1;
    iResult = ioctlsocket(this->listenSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR) 
    {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(this->listenSocket);
        WSACleanup();
        exit(1);
    }


    iResult = bind(this->listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) 
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(this->listenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    iResult = listen(this->listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) 
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(this->listenSocket);
        WSACleanup();
        exit(1);
    }

    this->getIpV4Str();
    this->getIpV6Str();

    struct pollfd poll;
    poll.fd = this->listenSocket;
    poll.events = POLLRDNORM;
    poll.revents = POLLRDNORM;

    this->polls.push_back(poll);
}

ServerNetwork::~ServerNetwork()
{
    for (std::map<SOCKET, unsigned int>::iterator it = sessions.begin(); it != sessions.end(); it++)
        closesocket(it->first);

    closesocket(this->listenSocket);
    WSACleanup();
}

void ServerNetwork::getIpV4Str()
{
    char str[sizeof(struct sockaddr_in)];

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    PCSTR pc = "";
    getaddrinfo(pc, DEFAULT_PORT, &hints, &result);
    inet_ntop(AF_INET, &(((struct sockaddr_in*)result->ai_addr)->sin_addr), str, sizeof(struct sockaddr_in));
    freeaddrinfo(result);

    printf("Server up at address %s ", str);
}

void ServerNetwork::getIpV6Str()
{
    char str[sizeof(struct sockaddr_in6)];

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    PCSTR pc = "";
    getaddrinfo(pc, DEFAULT_PORT, &hints, &result);
    inet_ntop(AF_INET6, &(((struct sockaddr_in6*)result->ai_addr)->sin6_addr), str, sizeof(struct sockaddr_in6));
    freeaddrinfo(result);

    printf("and %s\n", str);
}

bool ServerNetwork::acceptNewClient(unsigned int id)
{
    this->clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket != INVALID_SOCKET)
    {
        //disable nagle on the client's socket
        char value = 1;
        setsockopt(this->clientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

        char buffer[sizeof(Packet)];
        NetworkServices::receiveMessage(this->clientSocket, buffer, sizeof(Packet));
        
        Packet packet;
        packet.deserialize(buffer);

        std::string str = "";
        sendConnectedUser(str, this->clientSocket);

        Packet newUser;
        newUser.packet_type = NEW_USER;
        strcpy_s(newUser.pseudo, packet.pseudo);
        const unsigned int packet_size = sizeof(Packet);
        char packet_data[packet_size];
        newUser.serialize(packet_data);

        sendToAllExept(packet_data, sizeof(Packet), this->clientSocket);
        
        pseudos[this->clientSocket] = packet.pseudo;
        printf("%s has been connected to the server\n", packet.pseudo);

        sessions[this->clientSocket] = id;


        struct pollfd poll;
        poll.fd = this->clientSocket;
        poll.events = POLLRDNORM;
        poll.revents = POLLRDNORM;

        this->polls.push_back(poll);

        return true;
    }

    return false;
}

void ServerNetwork::receiveFromClient(SOCKET& socket)
{
    Packet packet;
    
    int data_length = receiveData(socket, network_data);

    int i = 0;
    while (i < data_length)
    {
        packet.deserialize(&(network_data[i]));
        i += sizeof(Packet);

        switch (packet.packet_type)
        {
        case INIT_CONNECTION:
            break;

        case DISCONNECT:
        {
            printf("%s has disconnected\n", packet.pseudo);
            sessions.erase(socket);
            pseudos.erase(socket);

            const unsigned int packet_size = sizeof(Packet);
            char packet_data[packet_size];

            packet.serialize(packet_data);

            sendToAll(packet_data, packet_size);
            break;
        }

        case MESSAGE:
        {
            printf("%s > %s\n", packet.pseudo, packet.message);

            const unsigned int packet_size = sizeof(Packet);
            char packet_data[packet_size];
            packet.serialize(packet_data);

            sendToAllExept(packet_data, packet_size, socket);
            break;
        }

        default:
            printf("error in packet types\n");
            break;
        }
    }
}

void Server::ServerNetwork::update()
{
    iResult = WSAPoll(this->polls.data(), (ULONG)this->polls.size(), -1);

    if (iResult == 0)
        return;

    if (polls[0].revents & POLLRDNORM)
        this->acceptNewClient(client_id++);

    for (size_t i = 1; i < polls.size(); i++)
    {
        if (polls[i].revents & POLLRDNORM)
            this->receiveFromClient(polls[i].fd);
        else if (polls[i].revents & POLLHUP)
        {
            sessions.erase(polls[i].fd);
            pseudos.erase(polls[i].fd);
            closesocket(polls[i].fd);
        }
    }
}

void Server::ServerNetwork::sendConnectedUser(std::string& str, SOCKET& socket)
{
    if (pseudos.size() == 0)
        return;

    Packet users;
    users.packet_type = ALREADY_USER;
    unsigned int i = 0;
    for (std::map<SOCKET, std::string>::iterator it = pseudos.begin(); it != pseudos.end(); it++, i++)
    {
        str += it->second;

        if (i < pseudos.size() - 1)
            str += " and ";
    }
    if (i > 1)
        str += " are already connected";
    else
        str += " is already connected";

    strcpy_s(users.message, str.c_str());
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    users.serialize(packet_data);
    sendToOne(packet_data, sizeof(Packet), socket);
}

int ServerNetwork::receiveData(SOCKET& socket, char* recvbuf)
{
    iResult = NetworkServices::receiveMessage(socket, recvbuf, MAX_PACKET_SIZE);
    if (iResult == 0)
    {
        printf("Connection closed from client %i\n", client_id);
        closesocket(socket);
    }
    return iResult;
}

void ServerNetwork::sendToAll(char* packets, int totalSize)
{
    SOCKET currentSocket;
    int iSendResult;
    for (std::map<SOCKET, unsigned int>::iterator it = sessions.begin(); it != sessions.end(); it++)
    {
        currentSocket = it->first;
        iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(it->first);
        }

        int b = 1;
    }
}

void ServerNetwork::sendToAllExept(char* packets, int totalSize, SOCKET& client)
{
    SOCKET currentSocket;
    int iSendResult;

    for (std::map<SOCKET, unsigned int>::iterator it = sessions.begin(); it != sessions.end(); it++)
    {
        currentSocket = it->first;
        if (currentSocket == client)
            continue;

        iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(currentSocket);
        }
    }
}

void ServerNetwork::sendToOne(char* packets, int totalSize, SOCKET& client)
{
    int iSendResult;
    iSendResult = NetworkServices::sendMessage(client, packets, totalSize);

    if (iSendResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(client);
    }
}

std::map<SOCKET, unsigned int>& Server::ServerNetwork::getSessions()
{
    return this->sessions;
}

std::map<SOCKET, std::string>& Server::ServerNetwork::getPseudos()
{
    return this->pseudos;
}