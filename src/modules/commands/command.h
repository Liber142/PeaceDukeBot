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
	virtual void Execute(const dpp::slashcommand_t &Event) = 0;
};
