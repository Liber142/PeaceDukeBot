#pragma once

#include "../bot_core.h"

class IModule 
{
private:
    CBotCore* botCore;

public:
    IModule(CBotCore* botCore) : botCore(botCore) {}

    virtual void OnInit() {}
    virtual CBotCore* BotCore() { return botCore; }
};
