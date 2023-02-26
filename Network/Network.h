#pragma once
#include <winsock2.h>
#include <Windows.h>

#define MAX_PACKET_SIZE 100000
#define DEFAULT_PORT "8000"

namespace Network
{
	class NetworkServices
	{
	public:
		static int sendMessage(SOCKET curSocket, char* message, int messageSize);
		static int receiveMessage(SOCKET curSocket, char* buffer, int bufSize);
	};

    enum PacketTypes
    {
        INIT_CONNECTION,
        MESSAGE,
        NEW_USER,
        ALREADY_USER,
        DISCONNECT,
        SERVER_SHUTDOWN,
    };

    struct Packet
    {
        unsigned int packet_type;
        char pseudo[12];
        char message[512];
        
        void serialize(char* data)
        {
            memcpy(data, this, sizeof(Packet));
        }

        void deserialize(char* data)
        {
            memcpy(this, data, sizeof(Packet));
        }
    };
}