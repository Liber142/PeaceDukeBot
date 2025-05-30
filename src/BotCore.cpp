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

#include "../include/BotCore.h"
#include "../include/ClanApplication.h"
#include "../include/ConstAgr.h"
#include "../include/ModsVote.h"

BotCore::BotCore(std::string& token) : bot(token) 
{
	 bot.intents = dpp::i_default_intents 
                | dpp::i_message_content 
                | dpp::i_guild_members
                | dpp::i_guild_message_reactions;

	SetupEvent();
	RegisterButton();
	RegisterSlashCommands();

    std::cout << "DPP version: " << dpp::utility::version() << std::endl;

    //bot.on_log([](const dpp::log_t& event) 
    //{
    //std::cout << dpp::utility::loglevel(event.severity) << ": " << event.message << "\n";
    //});    

};

void BotCore::StartDataBase(std::string v_filePath, std::string m_filePath, std::string AplicationAceptedMessage, std::string AplicationRejectedMessage)
{
    v_db = new DataBase(v_filePath);
    m_db = new DataBase(m_filePath);

    ModsVote::Initialize(bot, v_db, m_db, AplicationAceptedMessage, AplicationRejectedMessage);
    std::cout << "Vote Init" << std::endl
              << "v_filePath: " << v_filePath << std::endl
              << "m_filePath: " << m_filePath << std::endl;
}

void BotCore::SetupEvent()
{
   bot.on_ready([this](const dpp::ready_t& event) 
   {
       	std::cout << "Бот запущен как: " << bot.me.username << "\n";
    });  

    bot.on_form_submit([&](const dpp::form_submit_t& event) 
    {
        if (event.custom_id == "clan_apply") 
            ModsVote::RegisterVote(bot, event);
    });

    bot.on_guild_member_add([this](const dpp::guild_member_add_t& event)
    {
        dpp::snowflake guild_id = event.added.guild_id;
        bot.guild_member_add_role(guild_id, event.added.user_id, DEFAULT_ROLE_ID);
    });
     bot.on_message_create([this](const dpp::message_create_t& event)
     {
        if (event.msg.channel_id == CHANNEL_BOT_CLIENT_ID && event.msg.author.id != bot.me.id)
        {
            std::string messange = event.msg.content;
            std::ofstream file("/home/liber/game/hui/build/hui.txt");
            file << messange;
            file.close();
            std::cout << "bc-" << event.msg.author.global_name << ": " << messange << std::endl;
        }
        bot.message_delete(event.msg.id, CHANNEL_BOT_CLIENT_ID);
     });

     bot.on_ready([this](const dpp::ready_t& event)
     {
        std::cout << "bot.on_ready([this](const dpp::ready_t& event)" << std::endl;
        const dpp::snowflake& channel = CHANNEL_BOT_CLIENT_ID;
        bot.messages_get(CHANNEL_BOT_CLIENT_ID, 0, 0, 0, 500, [this, channel](const dpp::confirmation_callback_t& callback)
        {
            std::cout << "bot.messages_get(CHANNEL_BOT_CLIENT_ID, 0, 0, 0, 100, [this, channel](const dpp::confirmation_callback_t& callback)" << std::endl;
            if (callback.is_error())
            {
                bot.log(dpp::ll_error, "Не удалось получить сообщения: " + callback.get_error().message);
                return;
            }
            dpp::message_map messages = std::get<dpp::message_map>(callback.value);

            for (const auto& [message_id, message] : messages)
            {
                std::cout << "for (const auto& [message_id " << std::to_string(message_id) << ", message] : messages)" << std::endl;
                if (message.author.id == bot.me.id)
                    continue;

                std::cout << "bot.message_delete(message_id, message.channel_id);" << std::endl;
                bot.message_delete(message_id, message.channel_id);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            std::cout << "bot.on_ready([this](const dpp::ready_t& event) COMPLETE" << std::endl;
        });
     });
}

void BotCore::RegisterSlashCommands()
{
	bot.on_slashcommand([&](const dpp::slashcommand_t& event) 
	{
   		if (event.command.get_command_name() == "apply") 
        {
            dpp::message msg(event.command.channel_id, "Чтобы подать заявку на вступление в клан - нажмите кнопку ниже и заполните все поля. Мы сделаем все возможное, чтобы обработать ее как можно быстрее!");
            
            msg.add_component(
                dpp::component().add_component(
                    dpp::component()
                        .set_label("Подать заявку")
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_id("apply_button")
        	    )
     	    );
        
    	    event.reply(msg);

            
        }
    });
}

void BotCore::RegisterButton()
{
	bot.on_button_click([&](const dpp::button_click_t& event) 
	{
  		if (event.custom_id == "apply_button") 
  		{
   			ClanApplication::ShowApplicationModal(event);
  		}
	});
}

void BotCore::Start()
{
	bot.start();
}
