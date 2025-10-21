#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <chrono>
#include <cstddef>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "ConstAgr.h"
#include "Online.h"
#include "Parsing.h"

void C_OnlineClanMember::Init(dpp::cluster& bot)
{
    // Инициализация - получаем последние сообщения и находим наше
    bot.messages_get(CHANNEL_ONLINE_MEMBERS_ID, 0, 0, 0, 50, [&bot, this](const dpp::confirmation_callback_t& cc)
    {
        if (cc.is_error()) 
        {
            bot.log(dpp::ll_error, "Failed to get messages: " + cc.get_error().message);
            return;
        }

        auto messages = std::get<dpp::message_map>(cc.value);
        dpp::snowflake our_message_id = 0;
        
        // Ищем сообщения от нашего бота
        for (const auto& [id, msg] : messages) {
            if (msg.author.id == bot.me.id) {
                if (our_message_id == 0) {
                    // Сохраняем первое найденное сообщение
                    our_message_id = id;
                    last_message_id = id;
                } else {
                    // Удаляем лишние сообщения (оставляем только одно)
                    bot.message_delete(id, CHANNEL_ONLINE_MEMBERS_ID);
                }
            }
        }

        // Если нашли существующее сообщение, редактируем его
        if (our_message_id != 0) {
            UpdateMessage(bot);
        } else {
            // Если сообщения нет, создаем новое
            CreateNewMessage(bot);
        }
    });

    // Запускаем таймер для обновления каждые 10 секунд
    update_timer = bot.start_timer([this, &bot](dpp::timer timer) {
        UpdateMessage(bot);
    }, 10);

    std::cout << "C_OnlineClanMember initialized with timer" << std::endl;
}

void C_OnlineClanMember::UpdateMessage(dpp::cluster& bot)
{
    std::cout << "Updating online members..." << std::endl;
    
    data = Parsing::GetOnlineClanMembers("https://master1.ddnet.org/ddnet/15/servers.json");
    if (!data.empty() && data.contains("addresses"))
    {
        if (data != lastData)
        {
            ParsData(data);
            dpp::message msg = CreateMsg();
            msg.set_channel_id(CHANNEL_ONLINE_MEMBERS_ID);

            if (last_message_id != 0) {
                // Редактируем существующее сообщение
                msg.id = last_message_id;
                bot.message_edit(msg, [this](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) {
                        last_message_id = 0;
                    }
                });
            } else {
                CreateNewMessage(bot);
            }
        }
    }
    lastData = data;
}

void C_OnlineClanMember::CreateNewMessage(dpp::cluster& bot)
{
    dpp::message msg = CreateMsg();
    msg.set_channel_id(CHANNEL_ONLINE_MEMBERS_ID);

    bot.message_create(
        msg,
        [this](const dpp::confirmation_callback_t& callback) {
            if (!callback.is_error()) {
                auto msg = callback.get<dpp::message>();
                last_message_id = msg.id;
            }
        });
}

void C_OnlineClanMember::ParsData(nlohmann::json data)
{
    std::vector<MsgData> newServers;
    
    for (const auto& server : data)
    {
        MsgData newServer;
        if (server.contains("addresses") && server["addresses"].is_array())
        {
            if (!server["addresses"].empty())
            {
                newServer.ip = server["addresses"][0].get<std::string>();
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
        newServers.push_back(newServer);
    }
    Servers = std::move(newServers);
}

dpp::message C_OnlineClanMember::CreateMsg()
{
    dpp::message msg;
    std::string strMsg;
    
    for (size_t i = 0; i < Servers.size(); ++i)
    {
        std::string players;
        std::string title = "# [" + Servers[i].serverName + "](" + Servers[i].connectUrl + ")\n -# ip: " + Servers[i].ip + "\n";

        players += "\n**Players:**\n";
        for (size_t j = 0; j < Servers[i].clientName.size(); j++)
        {
            players += "\t\t\t" + Servers[i].clientName[j] + "\n";
        }

        std::string mapName = "\t**Map:** " + Servers[i].mapName + "\n";

        strMsg += title + mapName + players;
        if (!(i == Servers.size() - 1))
            strMsg += "===================================\n";
    }
    msg.set_content(strMsg);
    return msg;
}
