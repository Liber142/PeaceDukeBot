#include "bot_core.h"

#include <iostream>

#include <dpp/cluster.h>

int main()
{
    char* token = std::getenv("DISCORD_TOKEN");
    if(!token)
    {
        std::cerr << "Missing env DISCORD_TOKEN" << std::endl; 
        return 1;
    }

    dpp::cluster bot(token);

    CBotCore BotCore(&bot);

    bot.start();
}
