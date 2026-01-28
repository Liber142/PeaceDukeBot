#pragma once

#include "module.h"
#include "commands/command.h"

class CCommandHandler : public IModule
{
public:
    CCommandHandler(CBotCore* pBotCore);
	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CCommandHandler"; }
private:
    std::vector<std::unique_ptr<ICommand>> m_vpCommands;
};
