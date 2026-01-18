#include "../bot_core.h"

class IModule 
{
private:
    CBotCore* botCore;

public:
    IModule(CBotCore* botCore) : botCore(botCore) {};

    virtual void OnInit() = 0;

    virtual CBotCore* BotCore() { return botCore; }
};
