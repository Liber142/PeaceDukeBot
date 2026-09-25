#include "find_tas.h"

#include <execution>
#include <mutex>
#include <iomanip>
#include <sstream>

#include <engine/logger.h>
#include <engine/console.h>
#include <engine/database.h>
#include <engine/data_strucs.h>

#include <cpr/cpr.h>

void CFindTas::OnInit()
{
}

void CFindTas::OnConsoleInit()
{
	Console()->Register("find_player", {"s"}, 0, [this](const CConsole::IResult &Result) { FindPlayer(Result.GetString(0)); }, "Find player by name");
	Console()->Register("find_map", {"s"}, 0, [this](const CConsole::IResult &Result) { FindMap(Result.GetString(0)); }, "Find map by name");
	Console()->Register("check_player", {"s"}, 0, [this](const CConsole::IResult &Result) { CheckPlayer(Result.GetString(0)); }, "Check TAS ranks on player by nickname");
	Console()->Register("check_clan", {""}, 0, [this](const CConsole::IResult &Result) { CheckClan(); }, "Check TAS ranks on player by nickname");
}

void CFindTas::CheckPlayer(const std::string &Nick)
{
	CPlayer Player;
	try {
		Player = GetPlayer(Nick);
	} catch (...) {
		CLogger::Error(Name(), "Player not found");
		return;
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

	if(TasRanks.size() <= 0)
		return;

	CLogger::Info(Name(), Player.m_Nick);

	CLogger::Info(Name(), "Num TAS ranks: " + std::to_string(TasRanks.size()));
	for(const auto &Finish : TasRanks)
	{
		std::stringstream Str;
		Str	<< std::left
		    << std::setw(24) << Finish.m_Name;
		CLogger::Info(Name(), Str.str());
	}
	CLogger::Info(Name(), "-----");
}

void CFindTas::CheckClan()
{
	auto Keys = DataBase()->GetKeys("clan_members");
	std::mutex Mutex;
	std::vector<std::string> Nicks;

	for(const auto& Key : Keys)
	{
		auto User = DataBase()->Load<SUserData>("clan_members", Key);
		if(User)
			Nicks.emplace_back(User->m_GameNick);
	}

	std::for_each(std::execution::par,
		Nicks.begin(), Nicks.end(),
		[&](const auto &Nick) {
			std::lock_guard<std::mutex> Lock(Mutex);
			CheckPlayer(Nick);
	});
	
	for(auto& Key : Keys)
	{
	}
}

bool CFindTas::IsTas(const CFinish &Finish)
{
	if(Finish.m_Server == "Fun")
		return false;

	return true;
}


void CFindTas::PrintPlayer(const CFindTas::CPlayer &Player) const
{
	CLogger::Info(Name(), Player.m_Nick);
	std::stringstream Str;
	Str << std::left 
		<< std::setw(24) << "Map";
	CLogger::Info(Name(), Str.str());
	Str.str("");

	for(const auto &Finish : Player.m_Finishes)
	{
		Str << std::left 
			<< std::setw(24) << Finish.m_Name;

		CLogger::Info(Name(), Str.str());
		Str.str("");
	}
}

void CFindTas::PrintMap(const CFindTas::CMap &Map) const
{
	CLogger::Info(Name(), "Map printing is no longer supported by this API backend.");
}

CFindTas::CMap CFindTas::GetMap(const std::string &MapName)
{
	return CMap{};
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

	try { j = nlohmann::json::parse(r.text); } 
	catch (const std::exception &e) { 
		CLogger::Error(Name(), e.what());	
		throw;
	}

	CPlayer Player = from_json(j, Player);
	Player.m_Nick = Nick;
	m_CachePlayer[Nick] = Player;
	return Player;
}

void CFindTas::FindMap(const std::string &MapName)
{
	CLogger::Warning(Name(), "Global map search is no longer available.");
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
	if(j.contains("deleted_ranks"))
	{
		for(const auto &RawFinish : j["deleted_ranks"])
		{
			CFinish Finish = from_json(RawFinish, Finish);
			Result.m_Finishes.emplace_back(std::move(Finish));
		}	
	}
	else 
		CLogger::Warning(Name(), "Missing deleted_ranks");
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

