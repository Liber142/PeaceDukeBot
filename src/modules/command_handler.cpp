#include "command_handler.h"

#include "commands/apply.h"
#include "commands/command.h"
#include "commands/test.h"

#include <string>

CCommandHandler::CCommandHandler(CBotCore *pBotCore) :
	IModule(pBotCore)
{
	CApply *Apply = new CApply(pBotCore);
	AddCommand("apply", Apply);
}

CCommandHandler::~CCommandHandler()
{
	for(SCommand *pCmd = m_pFirstCmd; pCmd;)
	{
		SCommand *pTmp = pCmd;
		pCmd = pCmd->m_pNext;
		delete pTmp;
	}

	m_pFirstCmd = nullptr;
}

void CCommandHandler::AddCommand(std::string Name, ICommand *pCommand)
{
	SCommand *pTmp = new SCommand;
	pTmp->m_Name = Name;
	pTmp->m_pCommand = pCommand;

	pTmp->m_pNext = m_pFirstCmd;
	m_pFirstCmd = pTmp;
}

void CCommandHandler::OnInit()
{
	try
	{
		for(SCommand *pCmd = m_pFirstCmd; pCmd; pCmd = pCmd->m_pNext)
		{
			pCmd->m_pCommand->Register();
		}
	}
	catch(const std::exception &e)
	{
		std::cerr << "[ERROR] " << e.what() << std::endl;
	}

	BotCore()->Bot()->on_slashcommand([this](const dpp::slashcommand_t &Event) {
		Execute(Event);
	});
}

void CCommandHandler::Execute(const dpp::slashcommand_t &Event)
{
	try
	{
		std::string Name = Event.command.get_command_name();
		for(SCommand *pCmd = m_pFirstCmd; pCmd; pCmd = pCmd->m_pNext)
		{
			if(Name == pCmd->m_Name && pCmd->m_pCommand)
			{
				pCmd->m_pCommand->Execute(Event);
				return;
			}
		}
	}
	catch(const std::exception &e)
	{
		std::cerr << "[ERROR] " << e.what() << std::endl;
	}
}
