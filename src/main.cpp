#include "bot_core.h"

#include <dpp/cluster.h>
#include <engine/logger.h>

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


class CKiller
{
public:
	CKiller(dpp::cluster &Bot) : m_Bot(Bot), m_pBot(&Bot) {Killer();}

private:
	void Killer()
	{
		m_Bot.on_log([](const dpp::log_t& Log) {
		
		if (Log.severity >= dpp::ll_error) {
			CLogger::Error("main", "[" + dpp::utility::loglevel(Log.severity) + "] " + Log.message);

			if (Log.message.find("Authentication failed") != std::string::npos || 
				Log.message.find("Connect error") != std::string::npos ||
				Log.severity == dpp::ll_critical) {
				CLogger::Error("main", "Error with authentication or connection");
				std::exit(1);
			}
		}
		});

		m_Bot.on_ready([&](const dpp::ready_t& Event) {
			m_Bot.stop_timer(m_ConnectionTimeoutTimer);
		});

		m_ConnectionTimeoutTimer = m_Bot.start_timer([](dpp::timer h) {
			CLogger::Error("main", "Timeout for connect");
			g_Work = false; 
			std::exit(1); 
		}, 45);
	}
	dpp::timer m_ConnectionTimeoutTimer;
	dpp::cluster &m_Bot;
	dpp::cluster *m_pBot;
};

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
	CKiller Killer(Bot);
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
