#pragma once

#include "../module.h"

#include <dpp/dpp.h>

class ICommand : public IModule
{
public:
    ICommand(CBotCore* pBotCore)
        : IModule(pBotCore)
    {
    }
    virtual void Register();
    virtual void Execute();
};
