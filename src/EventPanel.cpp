#include "../include/EventPanel.h"
#include "../include/ConstAgr.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>

dpp::snowflake panelChannelId = EVENT_PANEL_CHANNEL_ID;

EventPanel::EventPanel(dpp::cluster& bot)
{
	bot.on_ready([&bot](const dpp::ready_t& event)
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

		dpp::message msg(panelChannelId, "Вот тут кнопочки есть кстати");
		dpp::component actionRow;
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

	});
}