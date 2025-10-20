#include "EventCommand.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>

EventCommand::EventCommand(dpp::cluster& bot_instance) : bot(bot_instance) {};

void EventCommand::Execute(const dpp::slashcommand_t& event)
{
    if (/*event.command.get_issuing_user().is_certified_moderator()*/true)
    {
        dpp::message msg(event.command.channel_id, "\t\tЗдесь вы можете организовать свой ивент. \nПосле нажатия кнопки ниже вам будет необходимо заполнить параметры ивент\nВаш ивент будет опубликован в канале обьявлений после принятия модерацией\nТакже у вас будет возможность пинговать участников ивента");
                
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_label("Начать ивент")
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_primary)
                            .set_id("start_event")
                    )
                );
            
        event.reply(msg);    
    }
}

dpp::slashcommand EventCommand::Register()
{
    return  dpp::slashcommand("event", "Создаст сообщение с кнопкой для подачи заявки", bot.me.id);
}