#ifndef MACRO_CONFIG_SNOWFLAKE
#error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, Def, Save, Desc)
#define MACRO_CONFIG_STR(Name, ScriptName, Def, Save, Desc)
#endif

MACRO_CONFIG_SNOWFLAKE(ROLE_ID, "role_id", 1000, CFGFLAG_SAVE, "Desc for role id");
MACRO_CONFIG_SNOWFLAKE(DEFAULT_ROLE_ID, "default_role", 1466599773279617280, CFGFLAG_SAVE, "This role got new guild member");
MACRO_CONFIG_SNOWFLAKE(CLAN_MEMBER_ROLE_ID, "clan_role", 1466599773388804240, CFGFLAG_SAVE, "This role got all clan members");

//Peace Duke
MACRO_CONFIG_SNOWFLAKE(DEFAULT_GUILD_ID, "default_guild", 1466599773279617272, CFGFLAG_SAVE, "This guild for Peace Duke clan");
MACRO_CONFIG_SNOWFLAKE(APPLY_CHANNEL_ID, "apply_channel_id", 1445341911237005313,CFGFLAG_SAVE, "This channel for vote by applycation");
MACRO_CONFIG_STR(ClanTag, "clan_tag", "Peace Duke", CFGFLAG_SAVE, "Default clan tag");
MACRO_CONFIG_STR(About, "about", "Hello", CFGFLAG_SAVE, "Desc for about");
