#include "invite.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>

invite::invite(dpp::cluster& bot_instance) : bot(bot_instance) {};

void invite::Execute(const dpp::slashcommand_t& event)
{
	dpp::message msg(event.command.channel_id, "hui");
            
            msg.add_component(
                dpp::component().add_component(
                    dpp::component()
                        .set_label("Пойти нахуй")
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_primary)
                        .set_url("discord-752165779117441075://")
                        .set_id("apply_button")
        	    )
     	    );
        
    event.reply(msg);    
}

dpp::slashcommand invite::Register()
{
	return  dpp::slashcommand("invite", "Создаст сообщение с кнопкой", bot.me.id);
}