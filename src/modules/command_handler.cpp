#include "command_handler.h"

#include "commands/apply.h"
#include "commands/command.h"

#include <string>

CCommandHandler::CCommandHandler(CBotCore *pBotCore) :
	IModule(pBotCore)
{
    std::unique_ptr<CApply> Apply = std::make_unique<CApply>(pBotCore);
    m_vpCommands.emplace_back(std::move(Apply));
}

void CCommandHandler::OnInit()
{
    CLogger::Info("command_handler", "Register commands");
    for(auto& pCmd : m_vpCommands)
    {
        pCmd->Register();
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
        for(auto& pCmd : m_vpCommands)
		{
			if(Name == pCmd->Name())
			{
				pCmd->Execute(Event);
				return;
			}
		}
	}
	catch(const std::exception &e)
	{
		std::cerr << "[ERROR] " << e.what() << std::endl;
	}
}
