#include "command.h"

class Apply : public ICommand
{
public:
    Apply(CBotCore* botcore);
    void Register() override;
    void Execute() override;
};
