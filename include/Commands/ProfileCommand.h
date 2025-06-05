#pragma once
#include "ICommand.h"
#include "../DataBase.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/snowflake.h>

class ProfileCommand : public ICommand
{
private:
	dpp::cluster& bot;
	DataBase* db;

	dpp::user GetTargetUser(const dpp::slashcommand_t& event);
public:
	explicit ProfileCommand(dpp::cluster& bot_instance, DataBase* db_instance);
	void Execute(const dpp::slashcommand_t& event) override;
	dpp::slashcommand Register() override;
};