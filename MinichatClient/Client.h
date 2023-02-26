#pragma once
#include <winsock2.h>
#include <Windows.h>
#include "ClientNetwork.h"
#include "Network.h"
#include <map>
#include <string>

#define ESC 27
#define ENTER '\r'
#define SHIFT '\0'
#define BACKSPACE 8
#define TABULATION 9

namespace Client
{
    class ClientChat
    {
    private:
        unsigned int id = -1;
        std::string pseudo;
        ClientNetwork* network = nullptr;
        char network_data[MAX_PACKET_SIZE];
        
        HANDLE inH;
        HANDLE outH;
        CONSOLE_SCREEN_BUFFER_INFO cbsi;
        HANDLE handles[2];
        INPUT_RECORD irInBuf[512];
        std::string strBuff = "";

        unsigned int columns;

    public:
        ClientChat(std::string& pseudo);
        void init(std::string& adress);
        void disconnect();
        bool isConnected();
        ~ClientChat();

        void update();
        void sendMessage(std::string& str);
        void receiveMessage();
        void printInput();
        bool handleArrowInput(unsigned int idx);
        void printToConsole();
        void printToMiddle(const std::string& str);
        void printToRight(const std::string& str);
        ClientNetwork* getClientNetwork();

        std::string& getPseudo();

        template<typename ... Args>
        std::string string_format(const std::string& format, Args ... args)
        {
            int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
            char* str = new char[size_s];
            std::snprintf(str, size_s, format.c_str(), args ...);
            return std::string(str);
        }
    };
}