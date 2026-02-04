#include "member_manager.h"

#include <engine/config.h>
#include <bot_core.h>

void CMemberManager::OnInit()
{
    Bot()->on_guild_member_add([this](const dpp::guild_member_add_t &Event) {
            const dpp::snowflake GuildId = Event.added.guild_id;
            const dpp::snowflake UserId = Event.added.user_id;
            const dpp::snowflake RoleId = Config()->DEFAULT_ROLE_ID;
            
            Bot()->guild_member_add_role(GuildId, UserId, RoleId);
        });
}

void CMemberManager::OnConsoleInit()
{
}
