#include "../../include/Commands/ProfileCommand.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>

ProfileCommand::ProfileCommand(dpp::cluster& bot_instance) : bot(bot_instance) {};

void ProfileCommand::Execute(const dpp::slashcommand_t& event)
{
    
}

dpp::slashcommand ProfileCommand::Register()
{
	return  dpp::slashcommand("Profile", "Покажет профиль участника", bot.me.id)
        .add_option(dpp::command_option(dpp::co_sub_command, "user", "профиль какого участника показать?"));
}