
#include "pch.h"
#include "framework.h"
#include "Network.h"

using namespace Network;

int NetworkServices::sendMessage(SOCKET curSocket, char* message, int messageSize)
{
	return send(curSocket, message, messageSize, 0);
}

int NetworkServices::receiveMessage(SOCKET curSocket, char* buffer, int bufSize)
{
	return recv(curSocket, buffer, bufSize, 0);
}