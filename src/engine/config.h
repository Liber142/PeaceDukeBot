#pragma once

#include "console.h"
#include "logger.h"

#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

class CConfig
{
	std::string CONFIG_FILE = "config.cfg";

public:
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, def, save, desc) \
	uint64_t Name = def;
#define MACRO_CONFIG_STR(Name, ScriptName, def, save, desc) \
	std::string Name = def;
#include "config_variables.h"
#undef MACRO_CONFIG_SNOWFLAKE
#undef MACRO_CONFIG_STR

	void OnInit(CConsole *pConsole)
	{
#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, def, save, desc) \
	{ \
		std::string name = ScriptName; \
		std::string help = desc; \
		pConsole->Register(name, {"i"}, save, [this, name](const CConsole::IResult &Result) { \
                    if(Result.NumArguments() > 0) { \
                     try\
                     {\
                        this->Name = std::stoull(Result.GetString(0)); \
                     }\
                     catch(...)\
                     {\
                        CLogger::Error("console", "wrong argument");\
                     }\
                    } \
                    CLogger::Info(name.c_str(), std::to_string(this->Name)); }, help); \
	}
#define MACRO_CONFIG_STR(Name, ScriptName, def, save, desc) \
	{ \
		std::string name = ScriptName; \
		std::string help = desc; \
		pConsole->Register(name, {"s"}, save, [this, name](const CConsole::IResult &Result) { \
                    if(Result.NumArguments() > 0) { \
                        this->Name = Result.GetString(0); \
                    } \
                    CLogger::Info(name.c_str(), this->Name.c_str()); }, help); \
	}
#include "config_variables.h"
#undef MACRO_CONFIG_SNOWFLAKE
#undef MACRO_CONFIG_STR

		pConsole->ExecuteFile(CONFIG_FILE);
	}
	~CConfig()
	{
		std::ofstream File(CONFIG_FILE);
		if(!File.is_open())
			return;

#define MACRO_CONFIG_SNOWFLAKE(Name, ScriptName, def, save, desc) \
	{ \
		std::string name = ScriptName; \
		if(this->Name != def && (save & CFGFLAG_SAVE)) \
		{ \
			File << name << " " << std::to_string(this->Name) << "\n"; \
		} \
	}

#define MACRO_CONFIG_STR(Name, ScriptName, def, save, desc) \
	{ \
		std::string name = ScriptName; \
		if(this->Name != def && (save & CFGFLAG_SAVE)) \
		{ \
			File << name << " " << this->Name << "\n"; \
		} \
	}

#include "config_variables.h"

#undef MACRO_CONFIG_SNOWFLAKE
#undef MACRO_CONFIG_STR
		File.close();
	}
};
