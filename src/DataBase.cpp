#include "../include/DataBase.h"

#include <nlohmann/json.hpp>
#include <dpp/snowflake.h>
#include <fstream>
#include <iostream>
#include <string>

void printHex(const std::string& s) 
{
    for (unsigned char c : s) 
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    std::cout << "\n";
}

DataBase::DataBase(const std::string& filepath) : filepath(filepath) 
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cout << "File: " << filepath << "not found." << std::endl;
        std::cout << "Create new file...." << std::endl;
        userData = nlohmann::json::object();
    }
    else
        file >> userData;
    
    std::cout << "File: " << filepath << std::endl;
    file.close();
    Save();
}

nlohmann::json DataBase::GetUser(const dpp::snowflake& userId) 
{
    return userData[std::to_string(userId)];
}

void DataBase::SetUser(const dpp::snowflake& userId, const nlohmann::json& newUserData) 
{
    this->userData[std::to_string(userId)] = newUserData;
}

void DataBase::Save() 
{
    //printHex(filepath);
    std::cout << "DataBase::Save( &" << &filepath << " )" << std::endl;
    std::cout << "DataBase::Save( " << filepath << " )" << std::endl;
    std::ofstream file(filepath);
    file << userData.dump(4);
    file.close();
}

void DataBase::SaveVoteData(const nlohmann::json& voteData) 
{
    std::cout << "SDataBase::SaveVoteData()" << std::endl;
    if (!userData.is_object())
        userData = nlohmann::json::object();
    
    userData["active_votes"] = voteData;
    Save();
}

nlohmann::json DataBase::GetVoteData() 
{
    std::cout << "SDataBase::GetVoteData" << std::endl;
    if (!userData.is_object()) 
        return nlohmann::json::object();

    if (userData.contains("active_votes")) 
        return userData["active_votes"];

    return nlohmann::json::object();
}

std::string DataBase::GetFilePath()
{
    return filepath;
}

const std::string* DataBase::p_GetFilePath()
{
    return &filepath;
}