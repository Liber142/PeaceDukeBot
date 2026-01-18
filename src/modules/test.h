#include "module.h"

class Test : public IModule
{
public:
    Test(CBotCore* botCore)
        : IModule(botCore) {};

    void OnInit() override;
};
