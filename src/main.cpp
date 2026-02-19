#include "bot_core.h"

#include <dpp/cluster.h>

#include <iostream>
#include <replxx.hxx>

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
		return 1;
	}

	dpp::cluster Bot(Token, dpp::i_default_intents | dpp::i_guild_members);

	CBotCore BotCore(&Bot);

	Bot.start(dpp::st_return);

	replxx::Replxx Replxx;
	Replxx.set_completion_callback([&BotCore](std::string const &Input, int &Pos) {
		std::vector<replxx::Replxx::Completion> Suggestions;

		if(Input.find(' ') != std::string::npos)
		{
			return Suggestions;
		}

		auto AllCommands = BotCore.Console()->GetAllCommands();
		Suggestions.reserve(AllCommands.size());

		for(auto const &Cmd : AllCommands)
		{
			if(Cmd.compare(0, Input.length(), Input) == 0)
			{
				Suggestions.emplace_back(Cmd);
			}
		}

		return Suggestions;
	});

	while(g_Work)
	{
		const auto &Input = Replxx.input("> ");
		if(!Input)
			break;

		std::string Line(Input);
		if(!Line.empty())
		{
			BotCore.Console()->ExecuteLine(Line);
			Replxx.history_add(Line);
		}
	}

	Bot.shutdown();
	return 0;
}
