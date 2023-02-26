#include "Server.h"

Server::ServerChat* server;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	delete server;
	return true;
}

int main()
{
	server = new Server::ServerChat();
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	while (true)
	{
		server->update();
	}

	return EXIT_SUCCESS;
}