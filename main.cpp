#include <dpp/dpp.h>
#include <cstdlib>
#include <dpp/guild.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <nlohmann/json_fwd.hpp>
#include <string>

#include "DataBase.h"
#include "BotCore.h"
#include "ConstAgr.h"

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
	auto config = load_config(PATH_CONFIG);
	std::string token = config.at("token").get<std::string>();

	BotCore bot(token);
    
    bot.Start();
}
