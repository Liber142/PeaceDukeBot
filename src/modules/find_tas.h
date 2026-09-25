#pragma once
#include "module.h"
#include <dpp/json.h>
#include <unordered_map>
#include <vector>
#include <string>

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

	struct CPlayer 
	{
		std::string m_Nick;
		std::vector<CFinish> m_Finishes;
	};

	struct CMap {};

	void OnInit() override;
	void OnConsoleInit() override;

	void PrintPlayer(const CPlayer &Player) const;
	void PrintMap(const CMap &Map) const;
	void CheckPlayer(const std::string &Nick);
	void CheckClan();
	bool IsTas(const CFinish &Finish);

	const std::string Name() const override { return "find_tas"; }

private:
	void FindPlayer(const std::string &Nick);
	void FindMap(const std::string &MapName);

	CPlayer GetPlayer(const std::string &Nick);
	CMap GetMap(const std::string &MapName);

	CFinish from_json(const nlohmann::json &j, const CFinish &Type) const;
	CPlayer from_json(const nlohmann::json &j, const CPlayer &Type) const;

	std::unordered_map<std::string, CPlayer> m_CachePlayer;
};

