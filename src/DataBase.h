#pragma once
#include <dpp/snowflake.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

class DataBase
{
private:
    const std::string filepath;
    nlohmann::json userData;

public:
    explicit DataBase(const std::string& filepath);

    nlohmann::json GetUser(const dpp::snowflake& userId);
    void SetUser(const dpp::snowflake& userId, const nlohmann::json& userData);
    void Save();
    void SaveVoteData(const nlohmann::json& voteData);
    nlohmann::json GetVoteData();
    std::string GetFilePath();
    const std::string* p_GetFilePath();
};
