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

    std::unique_ptr<dpp::cluster> Bot = std::make_unique<dpp::cluster>(Token);

	CBotCore BotCore(&Bot);

	Bot->start();
}
