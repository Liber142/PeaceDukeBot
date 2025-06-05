#include "../../include/Commands/Apply.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>

ApplyCommand::ApplyCommand(dpp::cluster& bot_instance) : bot(bot_instance) {};

void ApplyCommand::Execute(const dpp::slashcommand_t& event)
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

dpp::slashcommand ApplyCommand::Register()
{
	return  dpp::slashcommand("Apply", "Создаст сообщение с кнопкой для подачи заявки", bot.me.id);
}