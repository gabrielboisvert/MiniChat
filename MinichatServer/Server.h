#pragma once
#include "ServerNetwork.h"
#include "Network.h"

namespace Server
{
    class ServerChat
    {
    public:
        void update();

    private:
        ServerNetwork network;
    };
}