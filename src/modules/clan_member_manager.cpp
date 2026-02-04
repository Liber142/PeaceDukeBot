#include "clan_member_manager.h"
#include <bot_core.h>

#include <engine/database.h>
#include <engine/config.h>

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
    const dpp::snowflake GuildId = Config()->DEFAULT_GUILD_ID;
    const dpp::snowflake RoleId = Config()->CLAN_MEMBER_ROLE_ID;
    Bot()->guild_member_add_role(GuildId, Member.m_Id, RoleId);    

    Member.m_Clan = Config()->ClanTag;

    DataBase()->Save("clan_members", Member.m_Id, Member);
}
