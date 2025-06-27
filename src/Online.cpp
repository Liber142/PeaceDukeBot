#include <chrono>
#include <cstddef>
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
		 std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}

void C_OnlineClanMember::ParsData(nlohmann::json data)
{
	size_t i = 0;
	for (const auto& server : data)
	{
		MsgData newServer;
		if (server.contains("addresses") && server["addresses"].is_array()) 
		{
        	if (!server["addresses"].empty()) 
        	{
            	newServer.ip = server["addresses"][0].get<std::string>();
				std::cout << "5" << std::endl;
				newServer.connectUrl = "https://ddnet.org/connect-to/?addr=" + newServer.ip.substr(13);
            }
        }
		newServer.serverName = server["info"].value("name", "");
		newServer.mapName = server["info"]["map"].value("name", "");

		for (auto& client : server["info"]["clients"]) 
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

    	bool repeat;
    	for (size_t i = 0; i < Servers.size();i++ )
    	{
    		if (Servers[i].ip == newServer.ip)
    			repeat = true;
    	}
    	if (repeat)
    		Servers.push_back(newServer);
    	repeat = false;
    }
}

dpp::message C_OnlineClanMember::CreateMsg()
{
	dpp::message* msg = new dpp::message();
	//msg.set_flags(dpp::m_using_components_v2);
	std::string strMsg;
	for (size_t i = 0; i < Servers.size(); ++i)
	{
		std::string players;
		std::string title = "# [" + Servers[i].serverName 
							+ "](" + Servers[i].connectUrl 
							+ ")\n -# **ip:** " + Servers[i].ip;

		players += "\n**Players:**\n";
		for (size_t j = 0; j < Servers[i].clientName.size(); j++)
		{
			players += "\t\t\t" + Servers[i].clientName[j] + "\n";
		}

		std::string mapName = "\t**Map:**" + Servers[i].mapName + "\n";

		strMsg += title + mapName + players;
		if (!(i == Servers.size() - 1))
			strMsg += "===================================\n";
		/*
		msg.add_component_v2(
			dpp::component()
				.set_type(dpp::cot_separator)
				.set_spacing(dpp::sep_small)
				.set_divider(true)
				);*/
	}
	msg->set_content(strMsg);
	strMsg = "";
	dpp::message resultMsg = *msg;
	delete msg;
	return resultMsg;
}