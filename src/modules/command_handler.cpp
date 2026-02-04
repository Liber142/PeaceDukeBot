#include "command_handler.h"
#include <engine/logger.h>

CCommandHandler::CCommandHandler()
{
	m_vpCommands.insert(m_vpCommands.end(), {&m_Apply});
}

void CCommandHandler::OnModuleInit(CBotCore *pBotCore)
{
    IModule::OnModuleInit(pBotCore);
	for(const auto &pCommand : m_vpCommands)
        pCommand->OnModuleInit(pBotCore);
}
  
void CCommandHandler::OnInit()
{
	for(const auto &pCommand : m_vpCommands)
		pCommand->OnInit();
}

void CCommandHandler::OnConsoleInit()
{
	for(const auto &pCommand : m_vpCommands)
		pCommand->OnConsoleInit();
}
