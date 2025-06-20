#include <ctime>
#include <dpp/cluster.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>

class EventPanel
{
private:
	dpp::snowflake eventRole_id;
	dpp::snowflake channel_id;
	std::string eventEmorion;

	dpp::cluster& bot;

	void CreatePanelMsg(bool able);
public:
	EventPanel(dpp::cluster& bot);
	void ButtonHandler(const dpp::button_click_t& event);

	void SetEventRole(dpp::snowflake& role_id);
	void SetEventChannel(dpp::snowflake& channel_id);
	void SetEventEmotion(std::string& emotionid);
};