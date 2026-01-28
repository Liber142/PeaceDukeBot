#pragma once

#include "../module.h"

#include <dpp/dpp.h>

class ICommand : public IModule
{
public:
	ICommand(CBotCore *pBotCore) :
		IModule(pBotCore)
	{
	}
	virtual ~ICommand() = default;

	virtual void Register() = 0;
	virtual void Execute(CConsole::IResult& Result) = 0;
	std::string Name() const override { return "icommand"; }
protected: 
    CConsole& Console() { return BotCore()->Console(); };
};
