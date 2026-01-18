#pragma once

#include <stdint.h>
#include <string>

class CConfig 
{
    public: 
    #define MACRO_CONFIG_SNOWFLAKE(Name, def) \
        uint64_t Name = def; 
    #define MACRO_CONFIG_STR(Name, def) \
        std::string Name = def;
    #include "config_variables.h"
    #undef MACRO_CONFIG_SNOWFLAKE
    #undef MACRO_CONFIG_STR

};

extern CConfig g_Config;
