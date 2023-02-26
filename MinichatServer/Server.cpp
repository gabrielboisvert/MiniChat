#include "Server.h"
#include <string>

using namespace Server;
using namespace Network;

void ServerChat::update()
{
    this->network.update();
}