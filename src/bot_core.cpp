#include "bot_core.h"

#include "engine/config.h"
#include "engine/json_database.h"
#include "modules/command_handler.h"
#include "modules/module.h"

#include "git_revision.h"

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot),
	m_pConfig(std::make_unique<CConfig>()),
	m_pDataBase(std::make_shared<CJsonDataBase>()),
    m_pConsole(std::make_shared<CConsole>())
{
	m_pDataBase->Connect("db");

	// Init modules
	std::unique_ptr<CCommandHandler> CommandHandler = std::make_unique<CCommandHandler>(this);
	m_vpModules.emplace_back(std::move(CommandHandler));

	m_pBot->on_ready([this](const dpp::ready_t &Event) {
		CLogger::Info("botcore", m_pBot->me.format_username() + " ready!");
        CLogger::Info("botcore", "githash: " + std::string(GIT_SHORTREV_HASH));
		Init();
	});
}

CBotCore::~CBotCore() = default;

void CBotCore::Init()
{
	CLogger::Info("botcore", "Initial modules");
	for(const auto& pModule : m_vpModules)
	{
		pModule->OnInit();
        pModule->OnConsoleInit();
	}
}
