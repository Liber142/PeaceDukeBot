#pragma once

#include <dpp/cluster.h>

#include "engine/config.h"
#include "engine/database.h"

class CBotCore 
{
private:
    dpp::cluster* bot;
    
    CConfig* config;
    IDataBase* db;

public:  
    CBotCore(dpp::cluster* bot);

    CConfig* Config() { return config; }
    IDataBase* DataBase() { return db; }
    dpp::cluster* Bot() { return bot; }
};
