#pragma once

#include <engine/console.h>

#include <dpp/dpp.h>
#include <modules/module.h>

class ICommand : public IModule
{
public:
	virtual void Execute(CConsole::IResult &Result) = 0;
	const std::string Name() const override { return "icommand"; }
};
