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
}

void EventPanel::ButtonHandler(const dpp::button_click_t& event)
{
}

void EventPanel::CreatePanelMsg(bool able)
{
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