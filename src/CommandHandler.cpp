#include "../include/CommandHandler.h"
#include "../include/Commands/Apply.h"
#include "../include/Commands/ProfileCommand.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/stringops.h>
#include <memory>

CommandHandler::CommandHandler(dpp::cluster& bot) : bot(bot)
{
	commands["Apply"] = std::make_unique<ApplyCommand>(bot);
	commands["Profile"] = std::make_unique<ProfileCommand>(bot);
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
	auto it = commands.find(event.command.get_command_name());

	if (it != commands.end())
	{
		it->second->Execute(event);
		return true;
	}
	return true;
}


/*bot.on_ready([&bot](const dpp::ready_t & event) {
                if (dpp::run_once<struct register_bot_commands>()) {
                    dpp::slashcommand profile("profile", "Send a user profile.", bot.me.id);
                    profile.add_option(
                        
                        dpp::command_option(dpp::co_sub_command, "user", "Send a user profile.")
                            .add_option(dpp::command_option(dpp::co_user, "user", "User to turn into a dog.", false))
                    );
                    bot.global_command_create(profile);
                }
            });
*/