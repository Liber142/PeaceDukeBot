#pragma once

#include "commands/apply.h"
#include "module.h"

class CCommandHandler : public IModule
{
public:
	CCommandHandler();
	void OnModuleInit(class CBotCore *pBotCore) override;
	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CCommandHandler"; }

	//All commands
	CApplyCommand m_Apply;

private:
	std::vector<class ICommand *> m_vpCommands;
};
