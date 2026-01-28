#pragma once

#include <stdint.h>
#include <string>

#include "console.h"

class CConfig
{
public:
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, def, save, desc) \
	uint64_t Name = def; 
#define MACRO_CONFIG_STR(Name, ScriptName, def, save, desc) \
	std::string Name = def;
#include "config_variables.h"
#undef MACRO_CONFIG_SNOWFLAKE
#undef MACRO_CONFIG_STR
};

extern CConfig g_Config;


