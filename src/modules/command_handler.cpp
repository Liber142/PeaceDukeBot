#include "command_handler.h"

#include "commands/apply.h"

CCommandHandler::CCommandHandler(CBotCore *m_pBotCore) :
	IModule(m_pBotCore)
{
	CApply *Apply = new CApply(m_pBotCore);
	AddCommand("apply", Apply);
}

void CCommandHandler::AddCommand(std::string &Name, ICommand *pCommand)
{
	SCommand *pTmp = new SCommand;
	pTmp->m_Name = Name;
	pTmp->m_pCommand = pCommand;

	pTmp->m_pNext = m_pFirstCmd;
	pTmp = m_pFirstCmd;
}
