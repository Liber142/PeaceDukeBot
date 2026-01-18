#pragma once

#include "module.h"
#include "commands/command.h"

class CCommandHandler : public IModule
{
public:
	CCommandHandler(CBotCore *pBotCore);

	void OnInit() override;

private:
	struct SCommand
	{
		std::string m_Name;
		ICommand *m_pCommand = nullptr;
		SCommand *m_pNext = nullptr;
	} *m_pFirstCmd = nullptr;

	void AddCommand(std::string Name, ICommand *pCommand);
	void Execute(const dpp::slashcommand_t& Event);
};
