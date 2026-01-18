#include "bot_core.h"

#include "engine/config.h"
#include "modules/command_handler.h"

#include <iostream>

CBotCore::CBotCore(dpp::cluster *pBot) :
	m_pBot(pBot)
{
	m_pConfig = new CConfig();
	m_pDataBase = new CJsonDataBase();

	m_pDataBase->Connect("db");

	CCommandHandler *CommandHandler = new CCommandHandler(this);

	AddModule(CommandHandler);

	m_pBot->on_ready([this, CommandHandler](const dpp::ready_t &Event) {
		std::cout << "I'm " << m_pBot->me.format_username() << " ready!\n";
		Init();
	});
}

CBotCore::~CBotCore()
{
	delete m_pConfig;
	delete m_pDataBase;
	delete m_pBot;

	for(SModule *pCmd = m_pFirstModule; pCmd;)
	{
		SModule *pTmp = pCmd;
		pCmd = pCmd->m_pNext;
		delete pTmp;
	}
	m_pFirstModule = nullptr;
}

void CBotCore::AddModule(IModule *pModule)
{
	SModule *pTmp = new SModule;
	pTmp->m_pModule = pModule;
	pTmp->m_pNext = m_pFirstModule;
	m_pFirstModule = pTmp;
}

void CBotCore::Init()
{
	for(SModule *pModule = m_pFirstModule; pModule; pModule = pModule->m_pNext)
	{
		pModule->m_pModule->OnInit();
	}
}
