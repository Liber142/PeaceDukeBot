#include "find_tas.h"

#include <execution>
#include <mutex>

#include <engine/logger.h>
#include <engine/console.h>

#include <cpr/cpr.h>

void CFindTas::OnInit()
{
}

void CFindTas::OnConsoleInit()
{
	Console()->Register("find_player", {"s"}, 0, [this](const CConsole::IResult &Result) { FindPlayer(Result.GetString(0)); }, "Find player by name");
	Console()->Register("find_map", {"s"}, 0, [this](const CConsole::IResult &Result) { FindMap(Result.GetString(0)); }, "Find map by name");
	Console()->Register("check_player", {"s"}, 0, [this](const CConsole::IResult &Result) { CheckPlayer(Result.GetString(0)); }, "Check TAS ranks on player by nickname");
}

void CFindTas::CheckPlayer(const std::string &Nick)
{
	CPlayer Player;
	try {
		Player = GetPlayer(Nick);
	} catch (...) {
		CLogger::Error(Name(), "Player not found");
	}

	std::vector<CFinish> TasRanks;
	std::mutex Mutex;

	std::for_each(std::execution::par, 
		Player.m_Finishes.begin(), Player.m_Finishes.end(), 
		[&](const auto &Finish) {
			if(IsTas(Finish))
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				TasRanks.push_back(Finish);
			}
		});

	CLogger::Info(Name(), "Num TAS ranks: " + std::to_string(TasRanks.size()));
	for(const auto &Finish : TasRanks)
	{
		std::stringstream Str;
		Str	<< std::left
			<< std::setw(3) << Finish.m_Rank
		    << std::setw(24) << Finish.m_Name 
			<< std::setw(10) << Finish.m_Server;
		CLogger::Info(Name(), Str.str());
	}
}

bool CFindTas::IsTas(const CFinish &Finish)
{
	if(Finish.m_Server == "Fun")
		return false;

	CMap Map;
	try 
	{
		Map = GetMap(Finish.m_Name);
	}
	catch(...)
	{
		CLogger::Error(Name(), "Map not found");
		return false;
	}


	const int &TargetRank = Finish.m_Rank;
	std::vector<IRank*> TasRanks;
	int CountRanks = 0;
	if(Finish.m_IsTeamRank)
	{
		for(const auto &Rank : Map.m_TeamRanks)
		{
			if(Rank.m_Rank == TargetRank)
				CountRanks++;
		}
	}
	else 
	{
		for(const auto &Rank : Map.m_Ranks)
		{
			if(Rank.m_Rank == TargetRank)
				CountRanks++;
		}
	}
	 
	return CountRanks > 1;
}

void CFindTas::PrintPlayer(const CFindTas::CPlayer &Player) const
{
	CLogger::Info(Name(), Player.m_Nick);
	std::stringstream Str;
	Str << std::left 
		<< std::setw(24) << "Map"
		<< std::setw(10) << "Server"
		<< std::setw(5) << "Rank";
	CLogger::Info(Name(), Str.str());
	Str.str("");

	for(const auto &Finish : Player.m_Finishes)
	{
		Str << std::left 
			<< std::setw(24) << Finish.m_Name 
			<< std::setw(10) << Finish.m_Server 
			<< std::setw(5) << Finish.m_Rank;

		CLogger::Info(Name(), Str.str());
		Str.str("");
	}
}

void CFindTas::PrintMap(const CFindTas::CMap &Map) const
{
	std::stringstream Str;
	Str << std::left 
		<< std::setw(5) << "Rank"
		<< std::setw(24) << "Nick";
	CLogger::Info(Name(), Str.str());
	Str.str("");

	for(const auto &Rank : Map.m_TeamRanks)
	{
		Str << std::left 
			<< std::setw(5) << Rank.m_Rank
			<< std::setw(24) << Rank.m_Nicks[0];

		CLogger::Info(Name(), Str.str());
		Str.str("");

		if(Rank.m_Rank > 10)
			break;
	}
}

CFindTas::CMap CFindTas::GetMap(const std::string &MapName)
{
	if(m_CacheMap.contains(MapName))
		return m_CacheMap[MapName];

	cpr::Response r = cpr::Get(cpr::Url{"https://ddstats.tw/map/json"},
					  cpr::Parameters{{"map", MapName}});

	if(r.status_code / 100 != 2)
		throw;

	nlohmann::json j;

	try { j = nlohmann::json::parse(r.text); } 
	catch (const std::exception &e) { 
		CLogger::Error(Name(), e.what());	
		throw;
	}

	CMap Map = from_json(j, Map);
	m_CacheMap[MapName] = Map;
	return Map;
}

CFindTas::CPlayer CFindTas::GetPlayer(const std::string &Nick)
{
	if(m_CachePlayer.contains(Nick))
		return m_CachePlayer[Nick];

	cpr::Response r = cpr::Get(cpr::Url{"https://ddstats.tw/player/json"},
					  cpr::Parameters{{"player", Nick}});

	if(r.status_code / 100 != 2)
		throw;

	nlohmann::json j;

	try { j = nlohmann::json::parse(r.text); } 
	catch (const std::exception &e) { 
		CLogger::Error(Name(), e.what());	
		throw;
	}

	CPlayer Player = from_json(j, Player);
	m_CachePlayer[Nick] = Player;
	return Player;
}

void CFindTas::FindMap(const std::string &MapName)
{
	try 
	{
		CMap Map = GetMap(MapName);
		PrintMap(Map);
	}
	catch(...) 
	{
		CLogger::Warning(Name(), "Map not found");
	};
}

void CFindTas::FindPlayer(const std::string &Nick)
{
	try 
	{
		CPlayer Player = GetPlayer(Nick);
		PrintPlayer(Player);
	} 
	catch (...) 
	{
		CLogger::Error(Name(), "Player not found");
	}
}

CFindTas::CPlayer CFindTas::from_json(const nlohmann::json &j, const CPlayer &Type) const
{
	CPlayer Result;
	try
	{
		if(j.contains("profile") && j["profile"].contains("name"))
			Result.m_Nick = j["profile"].value("name", "");	
		else
		 	CLogger::Warning(Name(), "Missing player name");
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name() + " profile name", e.what());
	}

	if(j.contains("finishes"))
	{
		for(const auto &RawFinish : j["finishes"])
		{
			CFinish Finish = from_json(RawFinish, Finish);
			if(Finish.m_Rank > 0 && Finish.m_Rank <= 100)
				Result.m_Finishes.emplace_back(std::move(Finish));
		}	

		auto &Finishes = Result.m_Finishes;
		std::sort(Finishes.begin(), Finishes.end(),
			[](const CFinish &a, const CFinish &b) {
				return a.m_Rank < b.m_Rank;
			});
	}
	else 
		CLogger::Warning(Name(), "Missing finishes");
	return Result;
}

CFindTas::CMap 
CFindTas::from_json(const nlohmann::json &j, const CMap &Type) const
{
	CMap Result;
	try 
	{
		if(j.contains("rankings"))
		{
			for(const auto &JsonRank : j["rankings"])
			{
				CRank Rank = from_json(JsonRank, Rank);
				Result.m_Ranks.push_back(std::move(Rank));	
			}
		}
		if(j.contains("team_rankings"))
		{
			for(const auto &JsonRank : j["team_rankings"])
			{
				CTeamRank TeamRank = from_json(JsonRank, TeamRank);
				Result.m_TeamRanks.push_back(std::move(TeamRank));	
			}
		}
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name(), e.what());
	}
	return Result;
}

CFindTas::CRank 
CFindTas::from_json(const nlohmann::json &j, const CRank &Type) const
{
	CRank Result;
	try
	{
		if(j.contains("rank"))
			Result.m_Rank = j.value("rank", 0);
		else 
			CLogger::Warning(Name(), "Missing rank");

		if(j.contains("name"))
			Result.m_Nick = j.value("name", "");
		else 
			CLogger::Warning(Name(), "Missing name");
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name(), e.what());
	}
	return Result;
}

CFindTas::CTeamRank 
CFindTas::from_json(const nlohmann::json &j, const CTeamRank &Type) const
{
	CTeamRank Result;
	try
	{
		if(j.contains("rank"))
			Result.m_Rank = j.value("rank", 0);
		else 
			CLogger::Warning(Name(), "Missing rank");

		if(j.contains("players"))
		{
			for(const auto &Player : j["players"])
				Result.m_Nicks.emplace_back(Player);
		}
		else 
			CLogger::Warning(Name(), "Missing players");
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name(), e.what());
	}
	return Result;
}

CFindTas::CFinish 
CFindTas::from_json(const nlohmann::json &j, const CFinish &Type) const
{
	CFinish Result;
	try 
	{
		if(j.contains("map"))
		{
			if(j["map"].contains("map"))
				Result.m_Name = j["map"].value("map", "hui");
			else 
				CLogger::Warning(Name(), "Missing map name");
			if(j.contains("server"))
				Result.m_Server = j["map"].value("server", "hui");
			else 
				CLogger::Warning(Name(), "Missing map type");
		}
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name() + "_map", e.what());
	}

	try 
	{
		try 
        {
			if(j.contains("team_rank"))
			{
				Result.m_Rank = j.value("team_rank", 0);
				Result.m_IsTeamRank = true;
			}
		}	
		catch(const std::exception &e)
		{
			if(j.contains("rank"))
			{
				Result.m_Rank = j.value("rank", 0);
				Result.m_IsTeamRank = false;
			}
			else 
				CLogger::Warning(Name(), "Missing rank");
		}
	}
	catch(const std::exception &e)
	{
		CLogger::Error(Name() + "_rank", e.what());
	}

	return Result;
}
