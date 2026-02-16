#include "member_manager.h"

#include <engine/config.h>

#include <bot_core.h>

void CMemberManager::OnInit()
{
	Bot()->on_guild_member_add([this](const dpp::guild_member_add_t &Event) { HandleMemberAdd(Event); });

	Bot()->on_guild_member_update([this](const dpp::guild_member_update_t &Event) { HandleMemberUpdate(Event); });

	// WARNING: This method is limited to 1000 members per request.
	// If the server grows, a loop will be required to fetch all members via pagination.
	Bot()->guild_get_members(Config()->DEFAULT_GUILD_ID, 1000, 0,
		[this](const dpp::confirmation_callback_t &Callback) { SyncModerators(Callback); });
}

void CMemberManager::OnConsoleInit()
{
}

bool CMemberManager::IsModerator(const std::vector<dpp::snowflake> &Roles)
{
	return std::find(Roles.begin(), Roles.end(), Config()->MODER_ROLE_ID) != Roles.end();
}

bool CMemberManager::IsModerator(const dpp::snowflake &UserId)
{
	return std::find(m_vModers.begin(), m_vModers.end(), UserId) != m_vModers.end();
}

void CMemberManager::HandleMemberAdd(const dpp::guild_member_add_t &Event)
{
	Bot()->guild_member_add_role(Event.added.guild_id, Event.added.user_id, Config()->DEFAULT_ROLE_ID);
}

void CMemberManager::SyncModerators(const dpp::confirmation_callback_t &Callback)
{
	if(Callback.is_error())
	{
		CLogger::Error(Name(), Callback.get_error().human_readable);
		return;
	}

	m_NumModers = 0;
	m_vModers.clear();

	auto Member = std::get<dpp::guild_member_map>(Callback.value);
	for(const auto &[id, member] : Member)
	{
		if(IsModerator(member.get_roles()))
		{
			m_NumModers++;
			m_vModers.emplace_back(id);
		}
	}
}

void CMemberManager::HandleMemberUpdate(const dpp::guild_member_update_t &Event)
{
	dpp::snowflake UserId = Event.updated.user_id;
	auto It = std::find(m_vModers.begin(), m_vModers.end(), UserId);

	bool WasModer = (It != m_vModers.end());
	bool NowModer = IsModerator(Event.updated.get_roles());

	if(WasModer && !NowModer)
	{
		m_vModers.erase(It);
		m_NumModers--;
	}
	else if(!WasModer && NowModer)
	{
		m_vModers.emplace_back(UserId);
		m_NumModers++;
	}
}
