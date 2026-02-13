#include "votes.h"

#include <engine/database.h>
#include <engine/config.h>

#include <dpp/dpp.h>

void CApplyVoteManager::OnModuleInit(CBotCore *pBotCore) 
{
    IVoteManager::OnModuleInit(pBotCore);

    std::vector<size_t> Keys = DataBase()->GetKeys("clan_apply");
    for(const auto& Key : Keys)
    {
        auto Vote = DataBase()->Load<CClanVote>("clan_apply", Key);
        if(!Vote)
            continue;

        Vote->SetId(Key);
        std::unique_ptr<CClanVote> pVote = std::make_unique<CClanVote>(*Vote);
        pVote->OnModuleInit(BotCore());
        m_vpVotes[Key] = std::move(pVote);
    }
}

void CApplyVoteManager::OnInit()
{
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
            CLogger::Info(Name(), "Id: " + std::to_string(Vote->Id()));
            CLogger::Info(Name(), "Nickname: " + User.m_GameNick);
            CLogger::Info(Name(), "Age: " + std::to_string(User.m_Age));
            CLogger::Info(Name(), "About: " + User.m_About);

            CLogger::Info(Name(), "Yes: " + std::to_string(Vote->m_Yes) + " No: " + std::to_string(Vote->m_No));
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

    CLogger::Debug(Name(), Result.m_Name + " | " + Result.m_Args[0] + " | " + Result.m_Args[1]);

    if(!m_vpVotes.contains(Result.GetInt(0)))
    {
        CLogger::Error(Name(), "Vote not found");
        return;
    }


    if(Result.GetString(1) == "yes")
        m_vpVotes[Result.GetInt(0)]->AddVote(Result.m_Event->command.get_issuing_user().id, EVoteOptions::YES);
    else if(Result.GetString(1) == "no")
        m_vpVotes[Result.GetInt(0)]->AddVote(Result.m_Event->command.get_issuing_user().id, EVoteOptions::NO);
    Result.m_Event->reply();
    DataBase()->Save("clan_apply", m_vpVotes[Result.GetInt(0)]->Id(), *m_vpVotes[Result.GetInt(0)]);
}

void CApplyVoteManager::CClanVote::StartVote()
{
    m_State = EVoteState::PENDING;
    Bot()->message_create(GenerateMessage(), [this](const dpp::confirmation_callback_t &Callback) {
        if(Callback.is_error())
        {
            CLogger::Debug(Name(), "Create message " + Callback.get_error().human_readable);
            return;
        }
        m_MessageId = std::get<dpp::message>(Callback.value).id;
    });
}

void CApplyVoteManager::CClanVote::FinaleVote()
{

}

void CApplyVoteManager::CClanVote::AddVote(size_t VoterId, EVoteOptions Option)
{
    bool Found = false;
    for(auto& [Id, Vote] : m_vVotersIds)
    {
        if(Id == VoterId)
        {
            if(Vote != Option)
            {
                if(Vote == EVoteOptions::YES)
                { m_Yes--; m_No++; }
                else 
                { m_Yes++; m_No--; }
            }
            Vote = Option;
            Found = true;
        }
    }
    
    if(!Found)
    {
        m_vVotersIds.emplace_back(VoterId, Option);
        Option == EVoteOptions::YES ? m_Yes++ : m_No++;
    }

    Bot()->message_edit(GenerateMessage(), [this](const dpp::confirmation_callback_t &Callback) {
        if(Callback.is_error())
        {
            CLogger::Error(Name(), "Can't edit message: " + Callback.get_error().human_readable);
            Bot()->message_create(GenerateMessage(), [this](const dpp::confirmation_callback_t &Callback) {
                if(Callback.is_error())
                {
                    CLogger::Debug(Name(), "Create message " + Callback.get_error().human_readable);
                    return;
                }
                m_MessageId = std::get<dpp::message>(Callback.value).id;
            });
            return;
        }
        dpp::message Msg = std::get<dpp::message>(Callback.value);
        CLogger::Debug(Name(), "Success edit" + Msg.content);
    });
}

dpp::message CApplyVoteManager::CClanVote::GenerateMessage()
{
    dpp::user *User = dpp::find_user(m_TargetUser.m_Id); 
    if(!User)
    {
        CLogger::Error("Vote", "not found user");
        throw;
    }

    dpp::embed Embed;
    Embed.set_author(User->format_username(), "", User->get_avatar_url())
        .set_title("Новая заявка")
        .add_field("Ник: ", m_TargetUser.m_GameNick, true)
        .add_field("Возраст: ", std::to_string(m_TargetUser.m_Age), true)
        .add_field("О себе", m_TargetUser.m_About);

    switch(m_State)
    {
        case CApplyVoteManager::CClanVote::EVoteState::ACCEPTED: Embed.set_color(dpp::colors::green); break;
        case CApplyVoteManager::CClanVote::EVoteState::DECLINE: Embed.set_color(dpp::colors::red); break;
        default: Embed.set_color(dpp::colors::sti_blue); break;
    }
    
    if((m_No + m_Yes) != 0)
    {
        std::string Str;
        Str += "- :white_check_mark: За: " + std::to_string(m_Yes) + '\n';
        Str += "- :x: Против: " + std::to_string(m_No) + '\n';
        Embed.add_field("Голоса: ", Str);
    }

    dpp::message Msg(Config()->APPLY_CHANNEL_ID, Embed);
    

    if(m_State == EVoteState::PENDING)
    {
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
    }

    if(m_MessageId > 0)
        Msg.id = m_MessageId;

    return Msg;
}
