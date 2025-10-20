#pragma once
#include <dpp/dispatcher.h>
#include <dpp/snowflake.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "DataBase.h"
#include "IVoteSystem.h"

struct SApplicationVoteData
{
    dpp::snowflake m_targetUserId;
    dpp::snowflake m_processedBy;
    dpp::snowflake m_messageId;
    dpp::snowflake m_discussionChannelId;
    int m_Age;
    int m_SocialReting;
    std::string m_NickName;
    std::string m_About;
    std::string m_direckMessage;
    std::string m_status;
    std::string m_rejectionReason;
    std::chrono::system_clock::time_point m_decisionTime;
    bool m_isBlacklisted = false;

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
    void ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, 
                            SApplicationVoteData& application);
    void ShowEditModal(const dpp::button_click_t& event, SApplicationVoteData application);
    void ShowReasonModal(const dpp::button_click_t& event, SApplicationVoteData application);
    void ShowBlacklistModal(const dpp::button_click_t& event, SApplicationVoteData application);
    void ProcessModalResponse(const dpp::form_submit_t& event);
    void ProcessConfirmation(dpp::cluster& bot, const dpp::button_click_t& event, 
                           SApplicationVoteData& application, bool accepted);
    void ProcessFinalAcceptance(dpp::cluster& bot, SApplicationVoteData& application);
    void CreateDiscussionChannel(dpp::cluster& bot, const SApplicationVoteData& application);
    void ArchiveApplication(dpp::cluster& bot, SApplicationVoteData& application);
    void AssignMemberRole(dpp::cluster& bot, dpp::snowflake userId);
    void SendWelcomeMessage(dpp::cluster& bot, const SApplicationVoteData& application);
};
