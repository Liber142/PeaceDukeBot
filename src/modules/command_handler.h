#pragma once

#include "module.h"

class ICommand;

class CCommandHandler : public IModule
{
public:
	CCommandHandler(CBotCore *pBotCore);

	void OnInit() override;

private:
	struct SCommand
	{
		std::string m_Name;
		ICommand *m_pCommand;
		SCommand *m_pNext = nullptr;
	} *m_pFirstCmd;

	void AddCommand(std::string &Name, ICommand *pCommand);
	void Execute();
};
