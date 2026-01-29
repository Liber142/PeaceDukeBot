#ifndef MACRO_CONFIG_SNOWFLAKE
#error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, Def, Save, Desc)
#define MACRO_CONFIG_STR(Name, ScriptName, Def, Save, Desc)
#endif

MACRO_CONFIG_SNOWFLAKE(ROLE_ID, "role_id", 1000, CFGFLAG_SAVE, "Desc for role id");
MACRO_CONFIG_STR(About, "about", "Hello", CFGFLAG_SAVE, "Desc for about");
