#include "Client.h"
#include <string>
#include <iostream>
#include "main.h"
#include <windows.h>



Client::ClientChat* client = nullptr;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	delete client;
	return true;
}

int main()
{
	printf("Choose a pseudo ");
	std::string pseudo = "";
	std::cin >> pseudo;
	system("cls");

	client = new Client::ClientChat(pseudo);
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	std::string address = "";
	while (true)
	{
		while (!client->isConnected())
		{
			printf("Enter server adress ");
			std::cin >> address;
			system("cls");
			client->init(address);
			address = "";
		}

		client->update();
	}

	return 0;
}