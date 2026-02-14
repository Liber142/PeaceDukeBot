#include "bot_core.h"

#include "modules/module.h"

#include <engine/config.h>
#include <engine/console.h>
#include <engine/data_strucs.h>
#include <engine/database.h>
#include <engine/json_database.h>

#include <git_revision.h>

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot),
	m_pConfig(std::make_unique<CConfig>()),
	m_pDataBase(std::make_unique<CJsonDataBase>()),
	m_pConsole(std::make_unique<CConsole>())
{
	m_pDataBase->Connect("db");
	m_pConfig->OnInit(m_pConsole.get());

	m_vpModules.insert(m_vpModules.end(), {&m_CommandHandler,
						      &m_ClanMemberManager,
						      &m_ClanVoteManager});

	for(const auto &pModule : m_vpModules)
	{
		pModule->OnModuleInit(this);
		pModule->OnConsoleInit();
	}

	m_pBot->on_button_click([this](const dpp::button_click_t &Event) {
		m_pConsole->ExecuteInteraction(Event);
	});

	m_pBot->on_slashcommand([this](const dpp::slashcommand_t &Event) {
		m_pConsole->ExecuteInteraction(Event);
	});

	m_pBot->on_form_submit([this](const dpp::form_submit_t &Event) {
		m_pConsole->ExecuteInteraction(Event);
	});

	m_pBot->on_ready([this](const dpp::ready_t &Event) {
		Init();
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

CConfig *CBotCore::Config() const
{
	return m_pConfig.get();
}

IDataBase *CBotCore::DataBase() const
{
	return m_pDataBase.get();
}

CConsole *CBotCore::Console() const
{
	return m_pConsole.get();
}

dpp::cluster *CBotCore::Bot() const
{
	return m_pBot;
}
