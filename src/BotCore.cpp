#include <cctype>
#include <dpp/appcommand.h>
#include <dpp/cache.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/once.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>
#include <fstream>
#include <string>
#include <vector>

#include "BotCore.h"
#include "ClanApplication.h"
#include "ConstAgr.h"
#include "ModsVote.h"
#include "CommandHandler.h"
#include "RegisterEvent.h"

BotCore::BotCore(std::string& token) : bot(token), cmdHandler(bot)
{
	 bot.intents = dpp::i_default_intents 
                | dpp::i_message_content 
                | dpp::i_guild_members
                | dpp::i_guild_message_reactions;

	SetupEvent();
	RegisterButton();
	RegisterSlashCommands();


    ModsVote::Initialize(bot);
    RegisterEvent::Register(bot);


    std::cout << "DPP version: " << dpp::utility::version() << std::endl;


    bot.on_log([](const dpp::log_t& event) 
    {
    std::cout << dpp::utility::loglevel(event.severity) << ": " << event.message << "\n";
    });    

};

    //cmdHandler.Hui(m_db);

    //ModsVote::Initialize(bot, v_db, m_db, AplicationAceptedMessage, AplicationRejectedMessage);

void BotCore::SetupEvent()
{
   bot.on_ready([this](const dpp::ready_t& event) 
   {
       	std::cout << "Бот запущен как: " << bot.me.username << "\n";
        OnlineClanMember.Init(bot);
    });  


    bot.on_form_submit([&](const dpp::form_submit_t& event) 
    {
        if (event.custom_id == "clan_apply") 
            ModsVote::RegisterVote(bot, event);
    });

//Give degault role for newbee
    bot.on_guild_member_add([this](const dpp::guild_member_add_t& event)
    {
        dpp::snowflake guild_id = event.added.guild_id;
        bot.guild_member_add_role(guild_id, event.added.user_id, DEFAULT_ROLE_ID);
    });
}

void BotCore::RegisterSlashCommands()
{
	bot.on_ready([this](const dpp::ready_t& event)
    {
        cmdHandler.RegisterCommands();
    });

    bot.on_slashcommand([this](const dpp::slashcommand_t& event)
    {
        cmdHandler.HandleCommands(event);
    });
}

void BotCore::RegisterButton()
{
	bot.on_button_click([&](const dpp::button_click_t& event) 
	{
  		if (event.custom_id == "apply_button") 
   			ClanApplication::ShowApplicationModal(event);
	});
}

void BotCore::Start()
{
	bot.start();
}
