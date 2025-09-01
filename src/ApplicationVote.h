#pragma once
#include <unordered_map>
#include <unordered_set>
#include "DataBase.h"
#include "IVoteSystem.h"

struct SApplicationVoteData
{
    int m_voteAccept = 0;
    int m_voteReject = 0;
    nlohmann::json m_userData;
    std::unordered_set<dpp::snowflake> m_votedUsers;
    dpp::snowflake m_targetUserId;

    nlohmann::json ToJson() const;
    static SApplicationVoteData FromJson(const nlohmann::json& j);
};

class CApplicationVoteSystem : public IVoteSystem
{
public:
    void Initialize(dpp::cluster& bot) override;
    void ProcessButtonClick(const dpp::button_click_t& event) override;
    void ProcessFormSubmit(const dpp::form_submit_t& event) override;
    void SaveState() override;
    void LoadState() override;

private:
    std::unordered_map<dpp::snowflake, SApplicationVoteData> m_activeVotes;

    void CreateVoteMessage(dpp::cluster& bot, const dpp::user& user, const std::string& nickname, const std::string& age, const std::string& about, const std::string& points);
    void FinalizeVote(dpp::cluster& bot, const dpp::message& msg, SApplicationVoteData& vote, bool voteResult);
};
