#include "bot_core.h"

#include "engine/config.h"
#include "modules/command_handler.h"
#include "engine/json_database.h"

#include <iostream>

CBotCore::CBotCore(dpp::cluster *pBot) :
    m_pBot(pBot),
	m_pConfig(std::make_unique<CConfig>()),
	m_pDataBase(std::make_shared<CJsonDataBase>())
{
	m_pDataBase->Connect("db");

    // Init modules
    std::unique_ptr<IModule> CommandHandler = std::make_unique<CCommandHandler>(this);
    m_vpModules.emplace_back(std::move(CommandHandler));

	m_pBot->on_ready([this](const dpp::ready_t &Event) {
		std::cout << "I'm " << m_pBot->me.format_username() << " ready!\n";
		Init();
	});
}

void CBotCore::Init()
{
    for(auto It = m_vpModules.begin(); It != m_vpModules.end(); ++It)
    {
        It->get()->OnInit(); 
    }
}
