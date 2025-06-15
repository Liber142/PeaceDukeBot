#include "../include/EventPanel.h"
#include "../include/ConstAgr.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

dpp::snowflake panelChannelId = EVENT_PANEL_CHANNEL_ID;

EventPanel::EventPanel(dpp::cluster& bot) : bot(bot)
{
	bot.on_ready([&bot, this](const dpp::ready_t& event)
	{
		CreatePanelMsg(false);

	});
}

void EventPanel::ButtonHandler(const dpp::button_click_t& event)
{
    if (event.custom_id == "SetRoleEvent")
    {
        dpp::message msg(panelChannelId, "");
        
        msg.add_component(
            dpp::component().add_component(
                dpp::component()
                    .set_type(dpp::cot_role_selectmenu)
                    .set_placeholder("Выбери роль для ивента")
                    .set_max_values(1)
                    .set_id("selectrole")
            )
        );
        //msg.set_flags(dpp::m_ephemeral);
        event.reply(msg.set_flags(dpp::m_ephemeral));
    }
    else if (event.custom_id == "SetChannelEvent")
    {
        dpp::message msg(panelChannelId, "");
        
        msg.add_component(
            dpp::component().add_component(
                dpp::component()
                    .set_type(dpp::cot_channel_selectmenu)
                    .set_placeholder("Выбери канал для ивента")
                    .set_max_values(1)
                    .set_id("selectchanel")
            )
        );
        //msg.set_flags(dpp::m_ephemeral);
        event.reply(msg.set_flags(dpp::m_ephemeral));
    } 
    else if (event.custom_id == "EnableEventMode")
    {
        CreatePanelMsg(true);
        event.reply();
    }
    else if (event.custom_id == "DisableEventMode")
    {
        CreatePanelMsg(false);
        event.reply();
    }
}

void EventPanel::CreatePanelMsg(bool able)
{

    bot.messages_get(panelChannelId, 0, 0, 0, 1,[&](const dpp::confirmation_callback_t& event)
    {
       if (!event.is_error()) 
            {
           auto messages = std::get<dpp::message_map>(event.value);
                for (auto& message : messages) 
            {
                   bot.message_delete(message.second.id, message.second.channel_id);
                }
          }
      });

    sleep(1);

    dpp::message msg(panelChannelId, "Вот тут кнопочки есть кстати");
        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label(able ? "Вaключить" : "Включить")
                .set_type(dpp::cot_button)
                .set_style(able ? dpp::cos_danger : dpp::cos_success)
                .set_id(able ? "DisableEventMode" : "EnableEventMode")
            );
        actionRow.add_component(
            dpp::component()
                .set_label("роль")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("SetRoleEvent")
            );
        actionRow.add_component(
            dpp::component()
                .set_label("канал")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("SetChannelEvent")
            );
        actionRow.add_component(
            dpp::component()
                .set_label("эмоция")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("SetEmoteEvent")
            );
        msg.add_component(actionRow);
        bot.message_create(msg);
}
void EventPanel::SetEventRole(dpp::snowflake& role_id)
{

}

void EventPanel::SetEventChannel(dpp::snowflake& channel_id)
{

}

void EventPanel::SetEventEmotion(std::string& emotionid)
{

}