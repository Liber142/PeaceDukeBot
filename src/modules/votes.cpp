#include "votes.h"

#include "find_tas.h"

#include <engine/config.h>
#include <engine/database.h>

#include <bot_core.h>
#include <cpr/cpr.h>
#include <dpp/dpp.h>

#include <chrono>
#include <format>
#include <iterator>
#include <ranges>

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
		{
			try {
				Vote->SyncMessage(); 
			} catch (std::exception *e) {
				CLogger::Error(Name(), e->what());	
			}
		} }, "Update messsage for vote");
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

	Result.m_Event->thinking(true);

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
	Result.m_Event->delete_original_response();
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

	auto VoterId = Result.m_Event->command.get_issuing_user().id;
	if(Result.GetString(1) == "yes")
		m_vpVotes[Result.GetInt(0)]->AddVote(VoterId, EVoteOptions::YES);
	else if(Result.GetString(1) == "no")
		m_vpVotes[Result.GetInt(0)]->AddVote(VoterId, EVoteOptions::NO);
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
				{
					CLogger::Error(Name(), Callback.get_error().human_readable);
					return;
				}
				m_MessageId = std::get<dpp::message>(Callback.value).id;
			});
			return;
		}
		dpp::message Msg = std::get<dpp::message>(Callback.value);
	});
}

namespace
{
	void AddText(dpp::component *pBox, const std::string &Content)
	{
		pBox->add_component_v2(dpp::component()
				.set_type(dpp::cot_text_display)
				.set_content(Content));
	}

	void AddDivider(dpp::component *pBox)
	{
		pBox->add_component_v2(dpp::component()
				.set_type(dpp::cot_separator)
				.set_divider(true));
	}

	void AddButton(
		dpp::component *pRow, const std::string &Label,
		dpp::component_style Style, const std::string &Id)
	{
		pRow->add_component_v2(dpp::component()
				.set_type(dpp::cot_button)
				.set_label(Label)
				.set_style(Style)
				.set_id(Id));
	}
}

dpp::message CApplyVoteManager::CClanVote::GenerateMessage() const
{
	dpp::message Msg;
	Msg.channel_id = Config()->APPLY_CHANNEL_ID;
	Msg.set_flags(dpp::m_using_components_v2);

	dpp::component Box;
	FillContainer(&Box);
	Msg.add_component_v2(Box);

	if(m_State == EVoteState::PENDING)
		AddButtons(&Msg);

	if(m_MessageId > 0)
		Msg.id = m_MessageId;

	return Msg;
}

void CApplyVoteManager::CClanVote::FillContainer(dpp::component *pBox) const
{
	const dpp::user *User = dpp::find_user(m_TargetUser.m_Id);
	if(!User)
		throw std::runtime_error("User not found");

	pBox->set_type(dpp::cot_container);
	SetState(pBox, User->username);
	AddDivider(pBox);
	AddInfo(pBox);
	AddDivider(pBox);
	AddUrls(pBox);
	AddDivider(pBox);
	AddDeletedRanks(pBox);

	if(m_State == EVoteState::PENDING)
	{
		AddDivider(pBox);
		AddVotes(pBox);
	}
}

void CApplyVoteManager::CClanVote::SetState(
	dpp::component *pBox, const std::string &UserName) const
{
	using enum EVoteState;

	uint32_t Accent = 0x5865F2;
	std::string Title = "📩 Новая анкета";

	switch(m_State)
	{
	case ACCEPTED:
		Accent = dpp::colors::green;
		Title = "✅ Заявка одобрена";
		break;
	case DECLINE:
		Accent = dpp::colors::red;
		Title = "❌ Заявка отклонена";
		break;
	default:
		break;
	}

	pBox->set_accent(Accent);
	AddText(pBox, std::format("## {}\n**{}**", Title, UserName));
}

void CApplyVoteManager::CClanVote::AddInfo(dpp::component *pBox) const
{
	const std::string Info = [this] {
		std::string Info = std::format(
			"👤 **Ник:** {}", m_TargetUser.m_GameNick);
		if(m_TargetUser.m_Age.has_value())
			Info += std::format("  |  🎂 **Возраст:** {}", *m_TargetUser.m_Age);
		return Info + "\n" + m_TargetUser.m_About;
	}();

	AddText(pBox, Info);
}

void CApplyVoteManager::CClanVote::AddUrls(dpp::component *pBox) const
{
	const std::string Urls = [this] {
		cpr::CurlHolder Holder;
		std::stringstream Nick;
		Nick << Holder.urlEncode(m_TargetUser.m_GameNick);

		return std::format(
			"[DDNet](https://ddnet.org/players/{0})\n"
			"[DDStats](https://ddstats.tw/player/{0})",
			Nick.str());
	}();

	AddText(pBox, Urls);
}

void CApplyVoteManager::CClanVote::AddDeletedRanks(dpp::component *pBox) const
{
	constexpr int MaxShow = 5;

	const std::string &Nick = m_TargetUser.m_GameNick;
	const auto &Finishes = BotCore()->m_FindTas.Player(Nick).m_Finishes;

	const std::string Ranks = [&Finishes] {
		const auto Total = std::ssize(Finishes);

		std::string Ranks = std::format("**Удалённые карты** {}", Total);

		for(int i = 0; i < MaxShow && i < Total; i++)
			Ranks += "\n- " + Finishes[i].m_Name;

		if(Total > MaxShow)
			Ranks += std::format("\n- ... ещё {}", Total - MaxShow);

		return Ranks;
	}();

	AddText(pBox, Ranks);
}

void CApplyVoteManager::CClanVote::AddVotes(dpp::component *pBox) const
{
	AddText(pBox, std::format("✅ ` {} `  **|**  ❌ ` {} `", m_Yes, m_No));
}

void CApplyVoteManager::CClanVote::AddButtons(dpp::message *pMsg) const
{
	const std::string Id = std::format("apply_vote_manager_button {}", m_Id);

	dpp::component Row;
	Row.set_type(dpp::cot_action_row);
	AddButton(&Row, "Принять", dpp::cos_success, Id + " yes");
	AddButton(&Row, "Отклонить", dpp::cos_danger, Id + " no");
	pMsg->add_component_v2(Row);
}

std::optional<SBirthDate> CApplyVoteManager::ParseBirthday(const std::string &Input)
{
	std::tm TimeStruct = {};
	std::istringstream Stream(Input);

	Stream >> std::get_time(&TimeStruct, "%d.%m");

	if(Stream.fail())
	{
		CLogger::Error(Name(), "Stream.fail()");
		return std::nullopt;
	}

	SBirthDate Date;
	Date.m_Day = TimeStruct.tm_mday;
	Date.m_Month = TimeStruct.tm_mon + 1;

	int TempYear = 0;
	if(Stream.get() == '.' && (Stream >> TempYear))
	{
		if(TempYear < 100)
		{
			TempYear += (TempYear <= 25) ? 2000 : 1900;
		}
		Date.m_Year = TempYear;
	}

	return Date;
}

std::optional<int> CApplyVoteManager::CalculateAge(const SBirthDate &BirthDate)
{
	if(!BirthDate.m_Year.has_value())
	{
		return std::nullopt;
	}

	auto Now = std::chrono::system_clock::now();
	std::time_t NowTime = std::chrono::system_clock::to_time_t(Now);

	std::tm *CurrentTime = std::localtime(&NowTime);

	int CurrentYear = CurrentTime->tm_year + 1900;
	int CurrentMonth = CurrentTime->tm_mon + 1;
	int CurrentDay = CurrentTime->tm_mday;

	int Age = CurrentYear - *BirthDate.m_Year;

	if(CurrentMonth < BirthDate.m_Month || (CurrentMonth == BirthDate.m_Month && CurrentDay < BirthDate.m_Day))
	{
		Age--;
	}

	return Age;
}
