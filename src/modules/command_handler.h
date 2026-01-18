#include "module.h"

class ICommand;

class CCommandHandler : public IModule 
{
public:
    CCommandHandler(CBotCore* botCore);

    void OnInit() override;
private:
    struct SCommand 
    {
        std::string name;
        ICommand* pCommand;
        SCommand* next = nullptr;
    } *firstCmd;

    void AddCommand(std::string name, ICommand* pCommand);
    void Execute();
};
