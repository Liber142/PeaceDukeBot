#pragma once

#include "commands/command.h"
#include "module.h"

class CCommandHandler : public IModule
{
public:
	CCommandHandler(CBotCore *pBotCore);

	void OnInit() override;

	std::string Name() const override { return "CCommandHandler"; }

private:
	std::vector<std::unique_ptr<ICommand>> m_vpCommands;

	void AddCommand(std::string Name, ICommand *pCommand);
	void Execute(const dpp::slashcommand_t &Event);
};
