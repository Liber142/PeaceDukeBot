#pragma once

#include <modules/module.h>
#include <engine/console.h>

#include <dpp/dpp.h>

class ICommand : public IModule
{
public:
	virtual void Execute(CConsole::IResult &Result) = 0;
	const std::string Name() const override { return "icommand"; }
};
