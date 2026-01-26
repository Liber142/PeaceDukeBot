#include "bot_core.h"

#include "engine/config.h"
#include "engine/json_database.h"
#include "modules/command_handler.h"
#include "modules/module.h"

#include <iostream>

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot),
	m_pConfig(std::make_unique<CConfig>()),
	m_pDataBase(std::make_shared<CJsonDataBase>())
{
	m_pDataBase->Connect("db");

	// Init modules
	std::unique_ptr<CCommandHandler> CommandHandler = std::make_unique<CCommandHandler>(this);
	m_vpModules.emplace_back(std::move(CommandHandler));

	m_pBot->on_ready([this](const dpp::ready_t &Event) {
		CLogger::Info("botcore", m_pBot->me.format_username() + " ready!");
		Init();
	});
}

CBotCore::~CBotCore() = default;

void CBotCore::Init()
{
	CLogger::Info("botcore", "Initial modules");
	for(const auto &pSmartModule : m_vpModules)
	{
		IModule *pModule = pSmartModule.get();
		pModule->OnInit();
	}
}
