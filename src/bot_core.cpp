#include "bot_core.h"

#include "engine/config.h"
#include "engine/data_strucs.h"
#include "engine/json_database.h"
#include "git_revision.h"
#include "modules/command_handler.h"
#include "modules/module.h"

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot),
	m_pConfig(std::make_unique<CConfig>()),
	m_pDataBase(std::make_unique<CJsonDataBase>()),
	m_pConsole(std::make_unique<CConsole>())
{
	m_pDataBase->Connect("db");
	m_pConfig->OnInit(m_pConsole.get());

	// Init modules
	std::unique_ptr<CCommandHandler> CommandHandler = std::make_unique<CCommandHandler>(this);
	m_vpModules.emplace_back(std::move(CommandHandler));

	for(const auto &pModule : m_vpModules)
		pModule->OnConsoleInit();

	m_pBot->on_ready([this](const dpp::ready_t &Event) {
		Init();
	});

	m_pBot->on_button_click([this](const dpp::button_click_t &Event) {
		m_pConsole->ExecuteInteraction(Event);
	});

	m_pBot->on_slashcommand([this](const dpp::slashcommand_t &Event) {
		m_pConsole->ExecuteInteraction(Event);
	});
}

CBotCore::~CBotCore() = default;

void CBotCore::Init()
{
	CLogger::Info("botcore", m_pBot->me.format_username() + " ready!");
	CLogger::Info("botcore", "githash: " + std::string(GIT_SHORTREV_HASH));
	CLogger::Info("botcore", "Initial modules");

	for(const auto &pModule : m_vpModules)
	{
		pModule->OnInit();
	}
}
