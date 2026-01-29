#include "command_handler.h"

#include "commands/apply.h"

CCommandHandler::CCommandHandler(CBotCore *pBotCore) :
	IModule(pBotCore)
{
	std::unique_ptr<CApplyCommand> Apply = std::make_unique<CApplyCommand>(pBotCore);
	m_vpCommands.emplace_back(std::move(Apply));
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
