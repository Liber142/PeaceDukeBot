#include "database.h"
#include <fstream>
#include <iostream>

void JsonDataBase::Connect(std::string path)
{
    filePath = path + ".json";

    std::ifstream file(filePath);
    if(!file.is_open())
    {
        std::ofstream nfile(filePath);
        nfile << "{}";
        nfile.close();
    }
    else 
        file.close();
    Update();
}

UserData JsonDataBase::GetUser(uint64_t key)
{
    UserData result;
    try 
    {
        result = fromJson(json[key]);
        result.id = key;
    } 
    catch (std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
    }
    return result;
}

void JsonDataBase::AddUser(uint64_t key, UserData data)
{
    try 
    {
        nlohmann::json j = toJson(data);
        json["members"][std::to_string(key)] = j;
        Save();
    }
    catch (std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
    }

}

void JsonDataBase::Update()
{
    std::ifstream file(filePath);
    if(!file.is_open())
    {
        std::cerr << "Failed open db file" << std::endl;
        return;
    }

    try 
    {
        nlohmann::json j = nlohmann::json::parse(file); 
        file.close();
        json = j;
    }
    catch(std::exception& e)
    {
        std::cerr << "Error parse data base: "  << e.what() << std::endl;
    }
}

void JsonDataBase::Save()
{
    std::ofstream file(filePath);

    if(!file.is_open())
    {
        std::cerr << "Failed open db file" << std::endl;
        return;
    }

    try 
    {
        file << json.dump(4);
        file.close();
    }
    catch(std::exception& e)
    {
        std::cerr << "Error save data base file : "  << e.what() << std::endl;
    }
}

nlohmann::json JsonDataBase::toJson(UserData data)
{
    nlohmann::json result;

    result = {
        {"about", data.about},
        {"age", data.age},
        {"clan", data.clan},
        {"game_nick", data.gameNick},
        {"social_rating", data.socialRating}
    };

    return result;
}

UserData JsonDataBase::fromJson(nlohmann::json data)
{
    UserData result;

    result.age = data.value("age", 0);
    result.socialRating = data.value("social_rating", 0);
    result.gameNick = data.value("game_nick", "");
    result.clan = data.value("clan", "");
    result.about = data.value("about", "");


    return result;
}
