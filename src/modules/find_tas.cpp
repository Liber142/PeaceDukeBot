#include "find_tas.h"

#include <engine/console.h>
#include <engine/data_strucs.h>
#include <engine/database.h>
#include <engine/logger.h>

#include <cpr/cpr.h>

void CFindTas::OnInit()
{
}

void CFindTas::OnConsoleInit()
{
}

const CFindTas::CPlayer CFindTas::Player(const std::string &Nick)
{
	return GetPlayer(Nick);
}

CFindTas::CPlayer CFindTas::GetPlayer(const std::string &Nick)
{
	if(m_CachePlayer.contains(Nick))
		return m_CachePlayer[Nick];

	cpr::Response r = cpr::Get(cpr::Url{"https://ddnet.org/players/"},
		cpr::Parameters{{"json2", Nick}});

	if(r.status_code / 100 != 2)
		throw;

	nlohmann::json j;

	try
	{
		j = nlohmann::json::parse(r.text);
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name(), e.what());
		throw;
	}

	CPlayer Player = from_json(j, Player);
	Player.m_Nick = Nick;
	m_CachePlayer[Nick] = Player;
	return Player;
}

CFindTas::CPlayer CFindTas::from_json(const nlohmann::json &j, const CPlayer &Type) const
{
	CPlayer Result;
	if(j.contains("deleted_ranks"))
	{
		for(const auto &RawFinish : j["deleted_ranks"])
		{
			CFinish Finish = from_json(RawFinish, Finish);
			Result.m_Finishes.emplace_back(std::move(Finish));
		}
	}
	return Result;
}

CFindTas::CFinish CFindTas::from_json(const nlohmann::json &j, const CFinish &Type) const
{
	CFinish Result;
	Result.m_Rank = 0;
	Result.m_IsTeamRank = false;

	try
	{
		if(j.contains("map"))
			Result.m_Name = j.value("map", "hui");
		else
			CLogger::Warning(Name(), "Missing map name");

		if(j.contains("country"))
			Result.m_Server = j.value("country", "hui");
		else
			CLogger::Warning(Name(), "Missing country");
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name(), e.what());
	}

	return Result;
}
