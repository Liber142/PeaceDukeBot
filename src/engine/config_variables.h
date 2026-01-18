#ifndef MACRO_CONFIG_SNOWFLAKE
#    error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#    define MACRO_CONFIG_SNOWFLAKE(Name, Def)
#    define MACRO_CONFIG_STR(Name, Def)
#endif

MACRO_CONFIG_SNOWFLAKE(ROLE_ID, 1000);
MACRO_CONFIG_STR(about, "Hello");
