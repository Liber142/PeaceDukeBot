#pragma once
#include <dpp/nlohmann/json.hpp>
#include <dpp/snowflake.h>
#include <string>
#include <vector>

struct SUserData
{
	dpp::snowflake m_Id;
	int m_Age = 0;
	int m_SocialRating = 0;
	std::string m_GameNick;
	std::string m_Clan;
	std::string m_About;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SUserData, m_Id, m_Age, m_SocialRating, m_GameNick, m_Clan, m_About)
};

struct SVoteData
{
    SUserData m_User;
	int m_NumberVoteAccept = 0;
	int m_NumberVoteReject = 0;
	std::vector<std::string> m_vReasonsRejects;
	std::vector<dpp::snowflake> m_vVoters;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SVoteData, m_User, m_NumberVoteAccept, m_NumberVoteReject, m_vReasonsRejects, m_vVoters)
};

