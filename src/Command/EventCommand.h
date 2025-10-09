#pragma once
#include "ICommand.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

class EventCommand : public ICommand
{
private:
	dpp::cluster& bot;
public:
	explicit EventCommand(dpp::cluster& bot_instance);
	void Execute(const dpp::slashcommand_t& event) override;
	dpp::slashcommand Register() override;
};