#pragma once
#include "module.h"
#include <dpp/json.h>

class CFindTas : public IModule
{
public:
	struct CFinish
	{
		std::string m_Name;
		std::string m_Server;
		bool m_IsTeamRank;
		int m_Rank;
	};

	struct IRank 
	{
		int m_Rank;
	};

	struct CRank : IRank
	{
		std::string m_Nick;
	};

	struct CTeamRank : IRank
	{
		std::vector<std::string> m_Nicks;
	};

	struct CMap 
	{
		std::vector<CTeamRank> m_TeamRanks;
		std::vector<CRank> m_Ranks;
	};

	struct CPlayer 
	{
		std::string m_Nick;
		std::vector<CFinish> m_Finishes;
	};

	void OnInit() override;
	void OnConsoleInit() override;

	void PrintPlayer(const CPlayer &Player) const;
	void PrintMap(const CFindTas::CMap &Map) const;
	void CheckPlayer(const std::string &Nick);
	bool IsTas(const CFinish &Finish);

	const std::string Name() const override { return "find_tas"; }

private:
	void FindPlayer(const std::string &Nick);
	void FindMap(const std::string &MapName);

	CPlayer GetPlayer(const std::string &Nick);
	CMap GetMap(const std::string &MapName);

	CFinish from_json(const nlohmann::json &j, const CFinish &Type) const;
	CPlayer from_json(const nlohmann::json &j, const CPlayer &Type) const;
	CMap from_json(const nlohmann::json &j, const CMap &Type) const;
	CRank from_json(const nlohmann::json &j, const CRank &Type) const;
	CTeamRank from_json(const nlohmann::json &j, const CTeamRank &Type) const;

	std::unordered_map<std::string, CMap> m_CacheMap;
	std::unordered_map<std::string, CPlayer> m_CachePlayer;

};
