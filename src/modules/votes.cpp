#include "votes.h"

#include <engine/config.h>
#include <engine/database.h>

#include <bot_core.h>

#include <dpp/dpp.h>
#include <chrono>

void CApplyVoteManager::OnModuleInit(CBotCore *pBotCore)
{
	IVoteManager::OnModuleInit(pBotCore);

	std::vector<size_t> Keys = DataBase()->GetKeys(Name());
	for(const auto &Key : Keys)
	{
		auto Vote = DataBase()->Load<CClanVote>(Name(), Key);
		if(!Vote)
			continue;

		std::unique_ptr<CClanVote> pVote = std::make_unique<CClanVote>(*Vote);
		pVote->OnModuleInit(pBotCore);
		pVote->SetId(Key);

		pVote->OnFinale([this, Key]() {m_vpVotes.erase(Key); DataBase()->Erase(Name(), Key); });
		m_vpVotes[Key] = std::move(pVote);
	}
}

void CApplyVoteManager::OnInit()
{
}

void CApplyVoteManager::OnConsoleInit()
{
	Console()->Register(Name() + "_form", {"s", "i", "s"}, MODAL, [this](CConsole::IResult Result) { FormSubmit(std::move(Result)); }, "Serilizate form");
	Console()->Register(Name() + "_button", {"i", "s"}, MODAL, [this](CConsole::IResult Result) { ButtonClick(std::move(Result)); }, "Button click on vote");

#ifndef NDEBUG
	Console()->Register("update_vote", {}, 0, [this](CConsole::IResult Result) {
		for(const auto& [Key, Vote] : m_vpVotes)
			Vote->SyncMessage();
	}, "Update messsage for vote");
#endif
}

void CApplyVoteManager::FormSubmit(CConsole::IResult Result)
{
	if(Result.NumArguments() != 3 || !(Result.m_Flags & MODAL))
	{
		CLogger::Error("votes", "Wrong result");
		return;
	}
	std::string Nickname = Result.GetString(0);
	std::optional<SBirthDate> Birthday = ParseBirthday(Result.GetString(1));
	std::string About = Result.GetString(2);
	if(!Birthday.has_value())
	{
		Result.m_Event->reply(dpp::message("Неверная дата").set_flags(dpp::m_ephemeral));
		return;
	}

	SUserData User;
	User.m_GameNick = Nickname;
	User.m_BirthDay = Birthday.value();
	if(Birthday.value().m_Year.has_value())
		User.m_Age = CalculateAge(Birthday.value());
	User.m_About = About;
	User.m_Id = Result.m_Event->command.get_issuing_user().id;

	std::unique_ptr<CClanVote> Vote = std::make_unique<CClanVote>(User);
	Vote->OnModuleInit(BotCore());
	size_t Id = DataBase()->GenerateNewKey(Name());
	Vote->SetId(Id);
	Vote->StartVote();

	DataBase()->Save(Name(), Id, *Vote);
	Vote->OnFinale([this, Id]() {m_vpVotes.erase(Id); DataBase()->Erase(Name(), Id); });
	m_vpVotes[Id] = std::move(Vote);
	Result.m_Event->reply();
}

void CApplyVoteManager::ButtonClick(const CConsole::IResult Result)
{
	if(Result.NumArguments() != 2 || !(Result.m_Flags & BUTTON))
	{
		CLogger::Error(Name(), "Wrong result");
		return;
	}

	if(!BotCore()->m_ClanMemberManager.IsModerator(Result.m_Event->command.get_issuing_user().id))
	{
		Result.m_Event->reply(dpp::message("You have not permissions for this").set_flags(dpp::m_ephemeral));
		return;
	}

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

	if(m_vpVotes.contains(Result.GetInt(0)))
		DataBase()->Save(Name(), m_vpVotes[Result.GetInt(0)]->Id(), *m_vpVotes[Result.GetInt(0)]);
}

void CApplyVoteManager::CClanVote::StartVote()
{
	m_State = EVoteState::PENDING;
	SyncMessage();
}

void CApplyVoteManager::CClanVote::OnFinale(const std::function<void()> &Callback)
{
	m_CallBack = Callback;
}

void CApplyVoteManager::CClanVote::FinaleVote()
{
	SyncMessage();

	if(m_State == EVoteState::ACCEPTED)
	{
		Bot()->direct_message_create(m_TargetUser.m_Id, dpp::message(Config()->DIRECT_MESSAGE_APPROVE));
		dpp::message Msg(Config()->ClanChat, ""); 
		Msg.set_content("# <@" + std::to_string(m_TargetUser.m_Id) + "> " + Config()->CLAN_MESSAGE_APPROVE);
		Bot()->message_create(Msg);
		BotCore()->m_ClanMemberManager.AddClanMember(m_TargetUser);
	}
	else if(m_State == EVoteState::DECLINE)
		Bot()->direct_message_create(m_TargetUser.m_Id, dpp::message(Config()->DIRECT_MESSAGE_REJECT));

	m_CallBack();
}

void CApplyVoteManager::CClanVote::AddVote(const size_t &VoterId, const EVoteOptions &Option)
{

	bool Found = false;
	for(auto &[Id, Vote] : m_vVotersIds)
	{
		if(Id == VoterId)
		{
			if(Vote != Option)
			{
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

	if(m_Yes + m_No >= BotCore()->m_ClanMemberManager.NumModers())
	{
		if(m_No >= m_Yes)
		{
			m_State = EVoteState::DECLINE;
		}
		else
		{
			m_State = EVoteState::ACCEPTED;
		}
		FinaleVote();
		return;
	}

	SyncMessage();
}

void CApplyVoteManager::CClanVote::SyncMessage()
{
	Bot()->message_edit(GenerateMessage(), [this](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			Bot()->message_create(GenerateMessage(), [this](const dpp::confirmation_callback_t &Callback) {
				if(Callback.is_error())
					return;
				m_MessageId = std::get<dpp::message>(Callback.value).id;
			});
			return;
		}
		dpp::message Msg = std::get<dpp::message>(Callback.value);
	});

}

dpp::message CApplyVoteManager::CClanVote::GenerateMessage()
{
	dpp::message Msg(Config()->APPLY_CHANNEL_ID, GenerateEmbed());

	if(m_State == EVoteState::PENDING)
	{
		dpp::component ActionRow;
		ActionRow.add_component(
			dpp::component()
				.set_label("Принять")
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_success)
				.set_id("clan_apply_vote " + std::to_string(m_Id) + " yes"));
		ActionRow.add_component(
			dpp::component()
				.set_label("Отклонить")
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_danger)
				.set_id("clan_apply_vote " + std::to_string(m_Id) + " no"));
		Msg.add_component(ActionRow);
	}

	if(m_MessageId > 0)
		Msg.id = m_MessageId;

	return Msg;
}

dpp::embed CApplyVoteManager::CClanVote::GenerateEmbed()
{
    dpp::user *User = dpp::find_user(m_TargetUser.m_Id);
    if(!User) throw std::runtime_error("User not found");

    dpp::embed Embed;
    
    Embed.set_author(User->username, "", User->get_avatar_url());

    std::string Info = "👤 **Ник:** " + m_TargetUser.m_GameNick;
	if(m_TargetUser.m_Age.has_value())
		Info += "  |  🎂 **Возраст:** " + std::to_string(m_TargetUser.m_Age.value());
    
    Embed.set_description(Info + "\n\n" + m_TargetUser.m_About);

    switch(m_State)
    {
        case EVoteState::ACCEPTED: 
            Embed.set_color(dpp::colors::green)
                 .set_title("✅ Заявка одобрена"); 
            break;
        case EVoteState::DECLINE: 
            Embed.set_color(dpp::colors::red)
                 .set_title("❌ Заявка отклонена"); 
            break;
        default: 
            Embed.set_color(0x5865F2);
            Embed.set_title("📩 Новая анкета");
            
			std::string VoteBar = "✅ ` " + std::to_string(m_Yes) + " `  **|**  ❌ ` " + std::to_string(m_No) + " `";
            Embed.add_field("\u200b", VoteBar);
            break;
    }

    return Embed;
}

std::optional<SBirthDate> CApplyVoteManager::ParseBirthday(const std::string &Input)
{
    std::tm TimeStruct = {};
    std::istringstream Stream(Input);

    Stream >> std::get_time(&TimeStruct, "%d.%m");
    
    if (Stream.fail())
    {
		CLogger::Error(Name(), "Stream.fail()");
        return std::nullopt;
    }

    SBirthDate Date;
    Date.m_Day = TimeStruct.tm_mday;
    Date.m_Month = TimeStruct.tm_mon + 1;

    int TempYear = 0;
    if (Stream.get() == '.' && (Stream >> TempYear))
    {
        if (TempYear < 100)
        {
            TempYear += (TempYear <= 25) ? 2000 : 1900;
        }
        Date.m_Year = TempYear;
    }

    return Date;
}

std::optional<int> CApplyVoteManager::CalculateAge(const SBirthDate& BirthDate)
{
    if (!BirthDate.m_Year.has_value())
    {
        return std::nullopt;
    }

    auto Now = std::chrono::system_clock::now();
    std::time_t NowTime = std::chrono::system_clock::to_time_t(Now);
    
    std::tm* CurrentTime = std::localtime(&NowTime);
    
    int CurrentYear = CurrentTime->tm_year + 1900;
    int CurrentMonth = CurrentTime->tm_mon + 1;
    int CurrentDay = CurrentTime->tm_mday;

    int Age = CurrentYear - *BirthDate.m_Year;

    if (CurrentMonth < BirthDate.m_Month || (CurrentMonth == BirthDate.m_Month && CurrentDay < BirthDate.m_Day))
    {
        Age--;
    }

    return Age;
}

