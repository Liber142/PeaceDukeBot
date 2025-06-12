#include "../include/EventPanel.h"
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>

EventPanel::EventPanel(dpp::cluster& bot, const dpp::snowflake panelChannelId)
{
	bot.on_ready([&bot, panelChannelId](const dpp::ready_t& event)
	{
		bot.messages_get(panelChannelId, 0, 0, 0, 1,[&bot](const dpp::confirmation_callback_t& event)
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

		dpp::message msg = dpp::message();
		msg.set_content("Вот тут кнопочки чтобы ивенты настраивать.");
		dpp::component actionRow;
		actionRow.add_component(
            dpp::component()
                .set_label("роль")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("setEventRoleId")
            );
		actionRow.add_component(
            dpp::component()
                .set_label("канал")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("setEventChannelId")
            );
		actionRow.add_component(
            dpp::component()
                .set_label("эмоция")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("setEventChannelId")
            );

		msg.add_component(actionRow);
		
		msg.set_channel_id(panelChannelId);
		bot.message_create(msg);

	});
}