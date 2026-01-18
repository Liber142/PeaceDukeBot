#pragma once

#include <stdint.h>
#include <string>

#include <dpp/snowflake.h>
#include <dpp/nlohmann/json.hpp>

struct UserData
{
    dpp::snowflake id;

    int age;
    int socialRating;
    std::string gameNick;
    std::string clan;
    std::string about;
};

class IDataBase
{
public:
    virtual void Connect(std::string path) = 0;
    virtual UserData GetUser(uint64_t key) = 0;
    virtual void AddUser(uint64_t key, UserData data) = 0;
};

class JsonDataBase : public IDataBase
{
private:
    std::string filePath;
    nlohmann::json json;

    nlohmann::json toJson(UserData data);
    UserData fromJson(nlohmann::json data);

    void Update();
    void Save();

public:
    void Connect(std::string path) override;
    UserData GetUser(uint64_t key) override;
    void AddUser(uint64_t key, UserData data) override;
};
