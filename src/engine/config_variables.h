#ifndef MACRO_CONFIG_SNOWFLAKE
#error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, Def, Save, Desc)
#define MACRO_CONFIG_STR(Name, ScriptName, Def, Save, Desc)
#endif

MACRO_CONFIG_SNOWFLAKE(ROLE_ID, "role_id", 1000, CFGFLAG_SAVE, "General purpose role ID");
MACRO_CONFIG_SNOWFLAKE(DEFAULT_ROLE_ID, "default_role", 1466599773279617280, CFGFLAG_SAVE, "Role assigned to new guild members");
MACRO_CONFIG_SNOWFLAKE(CLAN_MEMBER_ROLE_ID, "clan_role", 1466599773388804240, CFGFLAG_SAVE, "Role for all verified clan members");
MACRO_CONFIG_SNOWFLAKE(MODER_ROLE_ID, "moder_role", 1472455735383162900, CFGFLAG_SAVE, "Role for guild moderators");

// Peace Duke
MACRO_CONFIG_SNOWFLAKE(DEFAULT_GUILD_ID, "default_guild", 1466599773279617272, CFGFLAG_SAVE, "Main guild ID for the Peace Duke clan");
MACRO_CONFIG_SNOWFLAKE(APPLY_CHANNEL_ID, "apply_channel_id", 1445341911237005313, CFGFLAG_SAVE, "Channel for application voting");
MACRO_CONFIG_SNOWFLAKE(ClanChat, "clan_chat", 0, CFGFLAG_SAVE, "Clan-only text channel");
MACRO_CONFIG_STR(ClanTag, "clan_tag", "Peace Duke", CFGFLAG_SAVE, "Default clan tag prefix");
MACRO_CONFIG_STR(About, "about", "Hello", CFGFLAG_SAVE, "General description of the clan");
MACRO_CONFIG_STR(DIRECT_MESSAGE_REJECT, "dr_message_reject", "Спасибо, что оставили заявку! К сожалению, мы вынуждены Вам отказать.😢 \n"
							     "Вы все еще можете оставаться в сообществе и общаться с другими участниками!\n"
							     "Надеюсь, Вы еще найдете место по душе!!🫶",
	CFGFLAG_SAVE, "DM sent when an application is rejected");

MACRO_CONFIG_STR(DIRECT_MESSAGE_APPROVE, "dr_message_approve", "Дорогой друг! Твоя заявка была одобрена!🎉\n"
							       "Добро пожаловать в наше мирное герцогство! С этого момента у тебя есть роль герцога и доступ к клановым каналам. Наши теги:\n"
							       " \tPeace Duke\n"
							       "\t‽eaceDuke\n"
							       "\t⚜‽Ð⚜\n"
							       "Выбирай любой!\n"
							       "Вливайся в общество и еще раз Добро пожаловать!🥳🎉",
	CFGFLAG_SAVE, "DM sent when an application is approved");

MACRO_CONFIG_STR(CLAN_MESSAGE_APPROVE, "clan_message_approve", "", CFGFLAG_SAVE, "Announcement sent to clan chat when a new member is accepted");
