#include "bot_core.h"

#include <dpp/cluster.h>

#include <iostream>

static std::atomic<bool> g_Work(true);

static void SignalHandler(int Sig)
{
	g_Work = false;
#ifdef _WIN32
	_close(0);
#else
	close(0);
#endif
}

int main()
{
	std::signal(SIGINT, SignalHandler);

	char *Token = std::getenv("DISCORD_TOKEN");
	if(!Token)
	{
		std::cerr << "Missing env DISCORD_TOKEN\n";
		delete Token;
		return 1;
	}

	dpp::cluster Bot(Token);

	CBotCore BotCore(&Bot);

	Bot.start(dpp::st_return);

	std::string Line;
	while(g_Work && std::getline(std::cin, Line))
	{
		if(!Line.empty())
		{
			BotCore.Console().ExecuteLine(Line);
		}
	}

	Bot.shutdown();
	return 0;
}
