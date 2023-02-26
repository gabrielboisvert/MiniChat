#include "Client.h"
#include "Network.h"
#include <windows.h>

using namespace Client;
using namespace Network;

ClientChat::ClientChat(std::string& pseudo) : pseudo(pseudo)
{
    this->inH = GetStdHandle(STD_INPUT_HANDLE);
    this->outH = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(outH, &cbsi);
    //rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    this->columns = cbsi.srWindow.Right - cbsi.srWindow.Left + 1;
}

void Client::ClientChat::init(std::string& adress)
{
    this->network = new ClientNetwork(adress);
    if (this->network->getSocket() == INVALID_SOCKET)
    {
        printf("Failed to connect\n");
        return;
    }
    
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = INIT_CONNECTION;
    strcpy_s(packet.pseudo, pseudo.c_str());
    packet.serialize(packet_data);

    NetworkServices::sendMessage(this->network->getSocket(), packet_data, packet_size);

    this->handles[0] = { network->getSockEvent() };
    this->handles[1] = { this->inH };
    printToMiddle("Successfully connected\n");
}

bool Client::ClientChat::isConnected()
{
    if (network == nullptr)
        return false;
    return network->isConnected();
}

void ClientChat::disconnect()
{
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    strcpy_s(packet.pseudo, pseudo.c_str());
    packet.packet_type = DISCONNECT;

    packet.serialize(packet_data);

    NetworkServices::sendMessage(this->network->getSocket(), packet_data, packet_size);
}

ClientChat::~ClientChat()
{
    this->disconnect();
}

void ClientChat::update()
{
    DWORD retval = WaitForMultipleObjects(2, this->handles, FALSE, INFINITE);
    switch (retval) {

        case WAIT_FAILED:
            printf("Failed to wait %d", GetLastError());
            return;

        case WAIT_OBJECT_0:
            this->network->extractEvents();

            if (this->network->getNetworkEvents().lNetworkEvents & FD_CLOSE)
            {
                this->printToMiddle("Server shutdown, you can try to reconnect\n");
                network->isConnected() = false;
            }
            else if (this->network->getNetworkEvents().lNetworkEvents & FD_READ)
                this->receiveMessage();
            break;

        case WAIT_OBJECT_0 + 1:
            this->printInput();
            break;
    }
}

void Client::ClientChat::sendMessage(std::string& str)
{
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = MESSAGE;
    strcpy_s(packet.pseudo, pseudo.c_str());
    strcpy_s(packet.message, str.c_str());
    packet.serialize(packet_data);

    NetworkServices::sendMessage(network->getSocket(), packet_data, packet_size);
}

void Client::ClientChat::printToMiddle(const std::string& str)
{
    SetConsoleTextAttribute(outH, 8);
    GetConsoleScreenBufferInfo(outH, &cbsi);
    int mid = (cbsi.srWindow.Right - cbsi.srWindow.Left + 1) / 2;

    mid -= (int)str.size() / 2;

    COORD coord = { (SHORT)mid, (SHORT)cbsi.dwCursorPosition.Y };
    SetConsoleCursorPosition(outH, coord);

    printf("%s\n", str.c_str());
}

void Client::ClientChat::receiveMessage()
{
    Packet packet;
    int data_length = network->receivePackets(network_data);

    if (data_length <= 0)
        return;

    int i = 0;
    while (i < data_length)
    {
        packet.deserialize(&(network_data[i]));
        i += sizeof(Packet);

        switch (packet.packet_type)
        {
        case INIT_CONNECTION:
            break;

        case ALREADY_USER:
            this->printToMiddle(this->string_format("%s\n", packet.message));
            break;

        case NEW_USER:
            this->printToMiddle(this->string_format("%s has entered the chat\n", packet.pseudo));
            break;

        case MESSAGE:
            this->printToRight(this->string_format("%s < %s\n", packet.message, packet.pseudo));
            break;

        case DISCONNECT:
            this->printToMiddle(this->string_format("%s has disconnected\n", packet.pseudo));
            break;

        case SERVER_SHUTDOWN:
            this->printToMiddle(this->string_format("Server has shutdown, new message will not be send\n"));
            break;

        default:
            printf("error in packet types\n");
            break;
        }
    }
}

void Client::ClientChat::printToRight(const std::string& str)
{
    SetConsoleTextAttribute(outH, 12);
    GetConsoleScreenBufferInfo(outH, &cbsi);
    int right = (int)cbsi.srWindow.Right - (int)str.size();

    COORD coord = { (SHORT)right, (SHORT)cbsi.dwCursorPosition.Y };
    SetConsoleCursorPosition(outH, coord);

    printf("%s", str.c_str());
}

void Client::ClientChat::printInput()
{
    ResetEvent(this->inH);

    DWORD retval;
    GetNumberOfConsoleInputEvents(this->inH, &retval);
    if (retval != 0)
    {
        ReadConsoleInputA(this->inH, this->irInBuf, 512, &retval);

        for (unsigned int i = 0; i < retval; i++)
        {
            switch (this->irInBuf[i].EventType)
            {
                case KEY_EVENT:
                    if (this->irInBuf[i].Event.KeyEvent.bKeyDown)
                    {
                        if (this->handleArrowInput(i))
                            continue;

                        char letter = irInBuf[i].Event.KeyEvent.uChar.AsciiChar;
                        switch (letter)
                        {
                            case SHIFT: break;

                            case ENTER:
                                if (this->strBuff.size() == 0)
                                    break;

                                this->sendMessage(this->strBuff);

                                this->strBuff = this->pseudo + " > " + this->strBuff;
                                this->printToConsole();
                                
                                printf("\n");
                                this->strBuff = "";
                                break;

                            case ESC:
                                delete this;
                                exit(EXIT_SUCCESS);

                            case TABULATION: break;

                            case BACKSPACE:
                            {
                                GetConsoleScreenBufferInfo(outH, &cbsi);

                                if (cbsi.dwCursorPosition.X == 0)
                                    break;

                                this->strBuff.erase(this->strBuff.begin() + (SHORT)cbsi.dwCursorPosition.X - 1);
                                printToConsole();

                                COORD coord = { (SHORT)cbsi.dwCursorPosition.X - 1, (SHORT)cbsi.dwCursorPosition.Y };
                                SetConsoleCursorPosition(outH, coord);
                                break;
                            }
                            default:
                                GetConsoleScreenBufferInfo(outH, &cbsi);

                                if (cbsi.dwCursorPosition.X < this->strBuff.size())
                                    this->strBuff.insert(this->strBuff.begin() + cbsi.dwCursorPosition.X, letter);
                                else
                                    this->strBuff += letter;

                                printToConsole();

                                COORD coord = { (SHORT)cbsi.dwCursorPosition.X + 1, (SHORT)cbsi.dwCursorPosition.Y };
                                SetConsoleCursorPosition(outH, coord);
                                break;
                            }
                    }
                    break;

            default:
                break;
            }
        }
    }
}

ClientNetwork* Client::ClientChat::getClientNetwork()
{
    return this->network;
}

std::string& Client::ClientChat::getPseudo()
{
    return this->pseudo;
}

bool Client::ClientChat::handleArrowInput(unsigned int idx)
{
    WORD key = this->irInBuf[idx].Event.KeyEvent.wVirtualKeyCode;
    LPPOINT point{ 0 };
    switch (key)
    {
        case VK_LEFT:
        {
            GetConsoleScreenBufferInfo(outH, &cbsi);
            COORD coord = { (SHORT)cbsi.dwCursorPosition.X - 1, (SHORT)cbsi.dwCursorPosition.Y };
            SetConsoleCursorPosition(outH, coord);
            return true;
        }
        case VK_RIGHT:
        {
            GetConsoleScreenBufferInfo(outH, &cbsi);
            if (cbsi.dwCursorPosition.X >= (SHORT)this->strBuff.size())
                return true;;

            COORD coord = { (SHORT)cbsi.dwCursorPosition.X + 1, (SHORT)cbsi.dwCursorPosition.Y };
            SetConsoleCursorPosition(outH, coord);
            return true;;
        }
        default:
            return false;
    }
}

void Client::ClientChat::printToConsole()
{
    SetConsoleTextAttribute(outH, 10);
    GetConsoleScreenBufferInfo(outH, &cbsi);

    SHORT tempX = cbsi.dwCursorPosition.X;
    COORD coord = { 0, (SHORT)cbsi.dwCursorPosition.Y };
    SetConsoleCursorPosition(outH, coord);

    SHORT length = (SHORT)this->strBuff.size();

    printf("\33[2K");
    printf("%s", this->strBuff.c_str());
    
    coord.X = length;

    SetConsoleCursorPosition(outH, coord);
}