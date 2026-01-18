#include "bot_core.h"

#include <iostream>

#include <dpp/cluster.h>

int main()
{
    char* Token = std::getenv("DISCORD_TOKEN");
    if (!Token)
    {
        std::cerr << "Missing env DISCORD_TOKEN" << std::endl;
        return 1;
    }

    dpp::cluster Bot(Token);

    CBotCore BotCore(&Bot);

    Bot.start();
}
