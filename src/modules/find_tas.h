#pragma once
#include "module.h"

#include <dpp/json.h>

#include <string>
#include <unordered_map>
#include <vector>

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

	struct CMap
	{
	};

	void OnInit() override;
	void OnConsoleInit() override;

	const CPlayer Player(const std::string &Nick);

	const std::string Name() const override { return "find_tas"; }

private:
	CPlayer GetPlayer(const std::string &Nick);

	CFinish from_json(const nlohmann::json &j, const CFinish &Type) const;
	CPlayer from_json(const nlohmann::json &j, const CPlayer &Type) const;

	std::unordered_map<std::string, CPlayer> m_CachePlayer;
};
