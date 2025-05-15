#include <dpp/dpp.h>
#include <cstdlib>
#include <dpp/nlohmann/json_fwd.hpp>
#include <fstream>
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

    std::string filePath = config.value("FilePath", "active_votes.json");
	BotCore bot(token);

    std::cout << "FilePath before bot.StartDataBase(filePath): " << filePath << std::endl;
    bot.StartDataBase(filePath);
    
    bot.Start();
}