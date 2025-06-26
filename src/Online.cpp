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

	std::cout << "Init(dpp::cluster& bot)" << std::endl;
	dpp::snowflake last_message_id = 0;
	while (true)
	{
		dpp::message msg;
		std::cout << "0" << std::endl;
		data = Parsing::GetOnlineClanMembers("https://master1.ddnet.org/ddnet/15/servers.json");
		std::cout << "1" << std::endl;
		if (!(data.empty() && data.contains("addresses")))
		{
			if (data != lastData)
			{
				ParsData(data);
				std::cout << "2" << std::endl;
				msg = CreateMsg();
				std::cout << "3" << std::endl;
				msg.set_channel_id(CHANNEL_ONLINE_MEMBERS_ID);

				if(last_message_id != 0) 
				{
                	bot.message_delete(last_message_id, CHANNEL_ONLINE_MEMBERS_ID);
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
	size_t i = 0;
	do 
	{
		MsgData newServer;
		std::cout << data.dump(2) << std::endl;
		std::cout << "4" << std::endl;
		if (data.contains("addresses") && data["addresses"].is_array()) 
		{
        	if (!data["addresses"].empty()) 
        	{
        		std::cout << "addresses content: " << data["addresses"].dump(2) << std::endl;
            	newServer.ip = data["addresses"][0].get<std::string>();
				std::cout << "5" << std::endl;
				newServer.connectUrl = "https://ddnet.org/connect-to/?addr=" + newServer.ip.substr(13);
            }
        }
		std::cout << "6" << std::endl;
		//newServer.serverName = data["info"].value("name", "");
		std::cout << "7" << std::endl;
		//newServer.mapName = data["info"]["map"].value("name", "");
		std::cout << "8" << std::endl;

		for (auto& client : data["info"]["clients"]) 
    	{
     	   if (client.contains("clan")) 
     	   {
       	 		for (const auto& clanTag : {"Peace Duke", "‽eaceDuke", "⚜‽Ð⚜"}) 
            	{
                	if (client["clan"] == clanTag) 
                	{
                		newServer.clientName.push_back(client["name"].get<std::string>());
                	    break; 
                	}
            	}
         	}
    	}
    	Servers.push_back(newServer);
    	++i;
    } while ((i < Servers.size()));
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