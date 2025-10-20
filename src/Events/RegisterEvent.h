#pragma once

#include <dpp/cluster.h>

class RegisterEvent
{
private:
	struct EventData 
	{
    	dpp::snowflake user_id;
    	dpp::snowflake event_role_id;       // Ивентовая роль
    	dpp::snowflake criteria_role_id;    // Роль-критерий
    	std::string event_time;             // Время проведения
    	std::string description;            // Описание ивента
	};

	time_t parseMoscowTime(const std::string& timeStr);
	std::string formatDiscordTime(time_t timestamp);

public:
	static void Register(dpp::cluster& bot);
};