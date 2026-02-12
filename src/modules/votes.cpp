#include "votes.h"

#include <engine/database.h>
#include <engine/config.h>

#include <dpp/dpp.h>

CApplyVoteManager::~CApplyVoteManager()
{
}

void CApplyVoteManager::OnInit()
{
    std::vector<size_t> Keys = DataBase()->GetKeys("clan_apply");
    for(const auto& Key : Keys)
    {
        auto Vote = DataBase()->Load<CClanVote>("clan_apply", Key);
        if(!Vote)
            continue;

        std::unique_ptr<CClanVote> pVote = std::make_unique<CClanVote>(*Vote);
        m_vpVotes[Key] = std::move(pVote);
    }
}

void CApplyVoteManager::OnConsoleInit()
{
    Console()->Register("clan_apply", {"s", "i", "s"}, MODAL, [this](CConsole::IResult Result) {FormSubmit(std::move(Result));}, "Serilizate form");
    Console()->Register("clan_apply_vote", {"i", "s"}, MODAL, [this](CConsole::IResult Result) {ButtonClick(std::move(Result));}, "Button click on vote");

    Console()->Register("check_vote", {}, 0, [this](CConsole::IResult Result) {
        if(m_vpVotes.empty())
        {
            CLogger::Info("Vote", "No one vote found");
            return;
        }
        for(const auto& [Key, Vote] : m_vpVotes)
        {
            SUserData User = Vote->GetUser();
            CLogger::Info("Vote", "Nickname: " + User.m_GameNick);
            CLogger::Info("Vote", "Age: " + std::to_string(User.m_Age));
            CLogger::Info("Vote", "About: " + User.m_About);

            CLogger::Info("Vote", "Yes: " + std::to_string(Vote->m_Yes) + " No: " + std::to_string(Vote->m_No));
        }
    }, "Test command for vote manager");
}

void CApplyVoteManager::FormSubmit(CConsole::IResult Result)
{
    if(Result.NumArguments() != 3 || !(Result.m_Flags & MODAL))
    {
        CLogger::Error("votes", "Wrong result");
        return;
    }
    std::string Nickname = Result.GetString(0);
    std::string AgeStr = Result.GetString(1);
    std::string About = Result.GetString(2);

    if (!std::all_of(AgeStr.begin(), AgeStr.end(), ::isdigit))
    {
        Result.m_Event->reply(dpp::message("Возраст должен быть числом").set_flags(dpp::m_ephemeral));
        return;
    }
    int Age = std::stoi(AgeStr);
    if (Age < 1)
    {
        Result.m_Event->reply(dpp::message("Некорректный возраст").set_flags(dpp::m_ephemeral));
        return;
    }

    SUserData User;
    User.m_GameNick = Nickname; 
    User.m_Age = Age;
    User.m_About = About;
    User.m_Id = Result.m_Event->command.get_issuing_user().id;

    std::unique_ptr<CClanVote> Vote = std::make_unique<CClanVote>(User);
    Vote->OnModuleInit(BotCore());
    Vote->SetId(DataBase()->GenerateNewKey("clan_apply"));
    Vote->StartVote();
    DataBase()->Save("clan_apply", Vote->Id(), *Vote);
    m_vpVotes[Vote->Id()] = std::move(Vote);
    Result.m_Event->reply();
}

void CApplyVoteManager::ButtonClick(const CConsole::IResult Result)
{
    if(Result.NumArguments() != 2)
    {
        CLogger::Error(Name(), "Wrong result");
        return;
    }

    if(Result.GetString(1) == "yes")
        m_vpVotes[Result.GetInt(0)]->AddVote(Result.m_Event->command.get_issuing_user().id, EVoteOptions::YES);
    else if(Result.GetString(1) == "no")
        m_vpVotes[Result.GetInt(0)]->AddVote(Result.m_Event->command.get_issuing_user().id, EVoteOptions::NO);
    Result.m_Event->reply();
    DataBase()->Save("clan_apply", m_vpVotes[Result.GetInt(0)]->Id(), *m_vpVotes[Result.GetInt(0)]);

    std::string Line = std::string("check_vote");
    Console()->ExecuteLine(Line);
}

void CApplyVoteManager::CClanVote::StartVote()
{
    dpp::user *User = dpp::find_user(m_TargetUser.m_Id); 
    if(!User)
    {
        CLogger::Error("Vote", "not found user");
        return;
    }

    dpp::embed Embed;
    Embed.set_author(User->format_username(), "", User->get_avatar_url())
        .set_title("Новая заявка")
        .set_color(dpp::colors::sti_blue)
        .set_title("Новая заявка")
        .add_field("Ник: ", m_TargetUser.m_GameNick)
        .add_field("Возраст: ", std::to_string(m_TargetUser.m_Age))
        .add_field("О себе", m_TargetUser.m_About);

    
    dpp::message Msg(Config()->APPLY_CHANNEL_ID, Embed);
    dpp::component ActionRow;
    ActionRow.add_component(
        dpp::component()
            .set_label("Принять")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_success)
            .set_id("clan_apply_vote " + std::to_string(m_Id) + " yes")
        );
    ActionRow.add_component(
        dpp::component()
            .set_label("Отклонить")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("clan_apply_vote " + std::to_string(m_Id) + " no")
        );
    Msg.add_component(ActionRow);

    Bot()->message_create(Msg, [this](const dpp::confirmation_callback_t &Callback) {
        if(Callback.is_error())
            return;
        m_MessageId = std::get<dpp::message>(Callback.value).id;
    });
}

void CApplyVoteManager::CClanVote::FinaleVote()
{

}

void CApplyVoteManager::CClanVote::AddVote(size_t VoterId, EVoteOptions Option)
{
    for(auto& [Id, Vote] : m_vVotersIds)
    {
        if(Id == VoterId)
        {
            if(Vote == Option)
                return;

            if(Vote == EVoteOptions::YES)
            {
                m_Yes--;
                m_No++;
            }
            else 
            {
                m_Yes++;
                m_No--;
            }
            Vote = Option;
            return;
        }
    }
    
    m_vVotersIds.emplace_back(VoterId, Option);
    Option == EVoteOptions::YES ? m_Yes++ : m_No++;
}
