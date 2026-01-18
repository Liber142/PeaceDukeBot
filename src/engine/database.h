#pragma once

#include <dpp/snowflake.h>
#include <stdint.h>

#include <dpp/nlohmann/json.hpp>
#include <string>

struct SUserData
{
	dpp::snowflake m_Id;

	int m_Age;
	int m_SocialRating;
	std::string m_GameNick;
	std::string m_Clan;
	std::string m_About;
};

class IDataBase
{
public:
	virtual ~IDataBase() = default;
	virtual void Connect(std::string Path) = 0;
	virtual SUserData GetUser(uint64_t Key) = 0;
	virtual void AddUser(uint64_t Key, SUserData Data) = 0;
};

