#pragma once
#include <dpp/cluster.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <unordered_map>
#include <unordered_set>

#include "DataBase.h"

struct VoteData
{
	int voteAccept = 0;
	int voteReject = 0;
	std::unordered_set<dpp::snowflake> votedUsers;
	nlohmann::json user;
	dpp::snowflake targedUserId;
	nlohmann::json to_json() const 
	{
		std::cout << "nlohmann::json to_json()" << std::endl;
		return 
		{
			{"voteAccept", voteAccept},
			{"voteReject", voteReject},
			{"votedUsers", votedUsers},
			{"targedUserId", targedUserId}
		};
	}

	static VoteData from_json(const nlohmann::json& j)
	{
		std::cout << "VoteData from_json()" << std::endl;
		VoteData v;
		v.voteAccept = j.value("voteAccept", 0);
		v.voteReject = j.value("voteReject", 0);
		v.targedUserId = j.value("targedUserId", 0);
		if (j.contains("votedUsers"))
		{
			for (const auto& id : j["votedUsers"])
			{
				v.votedUsers.insert(id.get<dpp::snowflake>());
			}
		}
		return v;
	}
};

class ModsVote
{
public:
	static void Initialize(dpp::cluster& bot, DataBase& db);
	static void RegisterVote(dpp::cluster& bot, const dpp::form_submit_t& event);
	static void LoadActiveVotes();
	static void SaveActiveVotes();
private:
	static std::unordered_map<dpp::snowflake, VoteData> activeVotes;
	static inline DataBase* voteDatabase;
};
