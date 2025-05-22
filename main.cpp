#include <dpp/dpp.h>
#include <cstdlib>
#include <dpp/guild.h>
#include <dpp/nlohmann/json_fwd.hpp>
#include <fstream>
#include <random>
#include <string>

#include "include/BotCore.h"
#include "include/DataBase.h"

nlohmann::json load_config(const std::string& path) 
{
    std::ifstream file(path);
    if (!file.is_open()) 
    {
        std::cerr << "Ошибка: файл " << path << " не найден!" << std::endl;
        return nullptr;
    }

    try 
    {
        nlohmann::json config;
        file >> config;
        return config;
    } 
    catch (const nlohmann::json::parse_error& e)
    {
        std::cerr << "Ошибка парсинга JSON: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() 
{
	auto config = load_config("config.json");
	std::string token = config.at("token").get<std::string>();

        std::string v_filepath = config.value("VoteFilePath", "active_votes.json");
        std::string m_filepath = config.value("MembersFilePath", "members_data.json");
        std::string AplicationAceptedMessage = config.value("AplicationAceptedMessage", "");
        std::string AplicationRejectedMessage = config.value("AplicationRejectedMessage", "");

	BotCore bot(token);

    bot.StartDataBase(v_filepath, m_filepath, AplicationAceptedMessage, AplicationRejectedMessage);
    
    bot.Start();
}