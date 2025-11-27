#pragma once
#include <dpp/dispatcher.h>
#include <dpp/snowflake.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include "DataBase.h"
#include "IVoteSystem.h"

struct SApplicationVoteData
{
    dpp::snowflake m_targetUserId;
    dpp::snowflake m_messageId;
    int m_Age = 0;
    int m_SocialReting = 0;
    std::string m_NickName;
    std::string m_About;
    std::string m_status = "pending";
    std::unordered_map<dpp::snowflake, bool> m_votes; // user_id -> vote (true = accept)
    
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
    std::string defaultAcceptedDirectMessage;
    std::string defaultRejectedDirectMessage;
    std::string defaultWelcomeMessage;
    std::unordered_map<dpp::snowflake, SApplicationVoteData> m_activeApplications;
    
    void CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, 
                                const std::string& nickname, const std::string& age, 
                                const std::string& about, const std::string& points);
    void ProcessVote(dpp::cluster& bot, const dpp::button_click_t& event, 
                    SApplicationVoteData& application, bool vote);
    void CheckVoteResult(dpp::cluster& bot, SApplicationVoteData& application);
    void AcceptApplication(dpp::cluster& bot, SApplicationVoteData& application);
    void RejectApplication(dpp::cluster& bot, SApplicationVoteData& application);
    void AssignMemberRole(dpp::cluster& bot, dpp::snowflake userId);
    void SendWelcomeMessage(dpp::cluster& bot, const SApplicationVoteData& application);
};
