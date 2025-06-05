#pragma once
#include "DataBase.h"
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <string>
#include <sys/types.h>
#include <unordered_map>

class ICommand;

class CommandHandler
{
public:
	CommandHandler(dpp::cluster& bot);

	void Hui(DataBase* db);
	~CommandHandler();
	void RegisterCommands();
	bool HandleCommands(const dpp::slashcommand_t& event);
private:
	DataBase* db;
	dpp::cluster& bot;
	std::unordered_map<std::string, std::unique_ptr<ICommand>> commands;
};