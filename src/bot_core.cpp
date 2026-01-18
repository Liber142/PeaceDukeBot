#include <iostream>

#include "bot_core.h"

#include "engine/config.h"
#include "modules/command_handler.h"

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot)
{
	m_pConfig = new CConfig();
	m_pDataBase = new CJsonDataBase();

	m_pDataBase->Connect("db");

    CCommandHandler *CommandHandler = new CCommandHandler(this);

    m_pBot->on_ready([this, CommandHandler](const dpp::ready_t& Event) {
        std::cout << "I'm " << m_pBot->me.format_username() << " ready!" << std::endl;

        CommandHandler->OnInit();
    });
}
