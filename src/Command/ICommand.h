#pragma once
#include <dpp/appcommand.h>
#include <dpp/dpp.h>

class ICommand
{
public:
	virtual ~ICommand() = default;
	virtual void Execute(const dpp::slashcommand_t& event) = 0;

	virtual dpp::slashcommand Register() = 0;
};