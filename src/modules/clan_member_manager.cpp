#include "clan_member_manager.h"

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
    const dpp::snowflake GuildId = BotCore()->Config().DEFAULT_GUILD_ID;
    const dpp::snowflake RoleId = BotCore()->Config().CLAN_MEMBER_ROLE_ID;
    Bot()->guild_member_add_role(GuildId, Member.m_Id, RoleId);    

    Member.m_Clan = BotCore()->Config().ClanTag;

    BotCore()->DataBase().Save("clan_members", Member.m_Id, Member);
}
