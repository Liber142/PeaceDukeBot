#pragma once
#include <unordered_map>
#include <unordered_set>
#include "DataBase.h"
#include "IVoteSystem.h"

struct SApplicationVoteData
{
    dpp::snowflake m_targetUserId;
    dpp::snowflake m_processedBy; // ID модератора, обработавшего заявку
    nlohmann::json m_userData;
    std::string m_status; // "pending", "accepted", "rejected"
    std::string m_rejectionReason; // Причина отказа
    
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
    std::unordered_map<dpp::snowflake, SApplicationVoteData> m_activeApplications;
    
    void CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, 
                                 const std::string& nickname, const std::string& age, 
                                 const std::string& about, const std::string& points);
    void ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, 
                             SApplicationVoteData& application);
    void ProcessAcceptance(dpp::cluster& bot, const dpp::button_click_t& event, 
                          SApplicationVoteData& application);
    void ProcessRejection(dpp::cluster& bot, const dpp::button_click_t& event, 
                         SApplicationVoteData& application);
    void ShowRejectionReasons(dpp::cluster& bot, const dpp::button_click_t& event, 
                             SApplicationVoteData& application);
    void FinalizeApplication(dpp::cluster& bot, const dpp::message& msg, 
                            SApplicationVoteData& application, bool accepted, 
                            const std::string& reason = "");
};
