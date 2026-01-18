#pragma once

#include "../module.h"

#include <dpp/dpp.h>

class ICommand : public IModule
{
public:
    ICommand(CBotCore* botcore)
        : IModule(botcore)
    {
    }
    virtual void Register();
    virtual void Execute();
};
