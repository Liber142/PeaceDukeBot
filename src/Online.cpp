#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <nlohmann/json.hpp>
#include <string>

#include "../include/Parsing.h"
#include "../include/Online.h"
#include "../include/ConstAgr.h"

void C_OnlineClanMember::Init(dpp::cluster& bot) 
{
	dpp::snowflake last_message_id = 0;
	while (true)
	{
		dpp::message msg;
		data = Parsing::GetOnlineClanMembers("https://master1.ddnet.org/ddnet/15/servers.json");
		if (!(data.empty() && data.contains("addresses")))
		{
			if (data != lastData)
			{
				ParsData(data);
				msg = CreateMsg();
				msg.set_channel_id(CHANNEL_MODERATION_ID);

				if(last_message_id != 0) 
				{
                	bot.message_delete(last_message_id, CHANNEL_MODERATION_ID);
            	}

            	bot.message_create(
                	dpp::message(msg),
                	[&last_message_id](const dpp::confirmation_callback_t& callback) 
                	{
                    	if(!callback.is_error()) 
                    	{
                        	auto msg = callback.get<dpp::message>();
                        	last_message_id = msg.id;
                    	}
                	}
            	);
			}
		}

		lastData = data;
		 std::this_thread::sleep_for(std::chrono::minutes(1));
	}
}

void C_OnlineClanMember::ParsData(nlohmann::json data)
{
	for (size_t i = 0; i < Servers.size(); ++i)
	{
		Servers[i].ip = data["addresses"][0].get<std::string>(); //62.122.215.19:8326
		Servers[i].connectUrl = "https://ddnet.org/connect-to/?addr=" + Servers[i].ip.substr(13);
		Servers[i].serverName = data["info"]["name"].get<std::string>();
		Servers[i].mapName = data["info"]["map"]["name"].get<std::string>();

		for (auto& client : data["info"]["clients"]) 
    	{
     	   if (client.contains("clan")) 
     	   {
       	 		for (const auto& clanTag : {"Peace Duke", "‽eaceDuke", "⚜‽Ð⚜"}) 
            	{
                	if (client["clan"] == clanTag) 
                	{
                		Servers[i].clientName.push_back(client["name"].get<std::string>());
                	    break; 
                	}
            	}
         	}
    	}
    }
}

dpp::message C_OnlineClanMember::CreateMsg()
{
	dpp::message msg;
	for (size_t i = 0; i < Servers.size(); ++i)
	{
		std::string players;
		std::string title = "# [" + Servers[i].serverName 
							+ "](" + Servers[i].connectUrl 
							+ ")\n";
		for (size_t j = 0; j < Servers[i].clientName.size(); j++)
		{
			players = "\t" + Servers[i].clientName[j] + "\n\t";
		}
		msg.set_content(title + players);
		msg.add_component_v2(
			dpp::component()
				.set_type(dpp::cot_separator)
				.set_spacing(dpp::sep_small)
				.set_divider(true)
				);
	}
	return msg;
}