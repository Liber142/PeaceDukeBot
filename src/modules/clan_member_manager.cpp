#include "clan_member_manager.h"

#include <engine/config.h>
#include <engine/database.h>

#include <bot_core.h>

void CClanMemberManager::OnInit()
{
	CMemberManager::OnInit();
}

void CClanMemberManager::OnConsoleInit()
{
	CMemberManager::OnConsoleInit();
}

void CClanMemberManager::AddClanMember(SUserData Member)
{
	Member.m_Clan = Config()->ClanTag;

	ChangeRole(Member);
	ChangeNick(Member);
	DataBase()->Save("clan_members", Member.m_Id, Member);
}

void CClanMemberManager::ChangeNick(const SUserData &Member)
{
	const auto &GuildId = Config()->DEFAULT_GUILD_ID;
	Bot()->guild_get_member(GuildId, Member.m_Id, [this, &GuildId, &Member](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			CLogger::Error(Name(), Callback.get_error().human_readable);
			return;
		}
		auto GuildMember = Callback.value;
		if(auto *Ptr = std::get_if<dpp::guild_member>(&GuildMember))
		{
			Ptr->set_nickname(Member.m_GameNick);
			Bot()->guild_edit_member(*Ptr, [this](const dpp::confirmation_callback_t &Callback) {
				if(Callback.is_error())
				{
					CLogger::Error(Name(), Callback.get_error().human_readable);
					return;
				}
			});
		}
	});
}

void CClanMemberManager::ChangeRole(const SUserData &Member)
{
	const size_t &GuildId = Config()->DEFAULT_GUILD_ID;
	const size_t &RoleId = Config()->CLAN_MEMBER_ROLE_ID;
	Bot()->guild_member_add_role(GuildId, Member.m_Id, RoleId, [this, &Member](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			CLogger::Error(Name(), Callback.get_error().human_readable);
			return;
		}

		CLogger::Info(Name(), "Succses add " + Member.m_GameNick + " to clan");
	});

	Bot()->guild_member_delete_role(GuildId, Member.m_Id, Config()->DEFAULT_ROLE_ID, [this](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			CLogger::Error(Name(), Callback.get_error().human_readable);
			return;
		}
	});
	Member.m_Clan = Config()->ClanTag;

	DataBase()->Save("clan_members", Member.m_Id, Member);
}
