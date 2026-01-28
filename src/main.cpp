#include "bot_core.h"

#include <dpp/cluster.h>

#include <iostream>

int main()
{
	char *Token = std::getenv("DISCORD_TOKEN");
	if(!Token)
	{
		std::cerr << "Missing env DISCORD_TOKEN" << std::endl;
		return 1;
	}

	dpp::cluster Bot(Token);

	CBotCore BotCore(&Bot);

	Bot.start(dpp::st_return);

    std::string Line;
    while(std::getline(std::cin, Line))
    {
        BotCore.Console().ExecuteLine(Line);
    }
}
