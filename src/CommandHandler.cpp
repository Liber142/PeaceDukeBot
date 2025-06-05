#include "../include/CommandHandler.h"
#include "../include/Commands/Apply.h"
#include "../include/Commands/ProfileCommand.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/stringops.h>
#include <memory>
#include <string>

CommandHandler::CommandHandler(dpp::cluster& bot, DataBase* db) : bot(bot), db(db)
{
	commands["Apply"] = std::make_unique<ApplyCommand>(bot);
	commands["profile"] = std::make_unique<ProfileCommand>(bot, db);
}

CommandHandler::~CommandHandler() = default;

void CommandHandler::RegisterCommands()
{
	for (auto& [name, cmd] : commands)
	{
		bot.global_command_create(cmd->Register());
	}
}

bool CommandHandler::HandleCommands(const dpp::slashcommand_t& event)
{
	std::cout << "CommandHandler::HandleCommands(const dpp::slashcommand_t& event)" << std::endl;

	auto it = commands.find(event.command.get_command_name());
	std::cout << "event.command.get_command_name()" << event.command.get_command_name() << std::endl;

	if (it != commands.end())
	{
		it->second->Execute(event);
		return true;
	}
	return true;
}