#pragma once
#include <string>
#include <vector>

#include <dpp/snowflake.h>

struct IData 
{
    virtual ~IData() {}
};

struct SUserData : IData
{
	dpp::snowflake m_Id;

	int m_Age;
	int m_SocialRating;
	std::string m_GameNick;
	std::string m_Clan;
	std::string m_About;
};

struct SVoteData : SUserData
{
    int m_NumberVoteAccept;
    int m_NumberVoteReject;

    std::vector<std::string> m_vReasonsRejects;
    std::vector<dpp::snowflake> m_vVoters;
};

