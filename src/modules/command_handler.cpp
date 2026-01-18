#include "command_handler.h"
#include "commands/apply.h"

CCommandHandler::CCommandHandler(CBotCore* botCore) : IModule(botCore)
{
    Apply* apply = new Apply(botCore);
    AddCommand("apply", apply);
}

void CCommandHandler::AddCommand(std::string name, ICommand* command)
{
    SCommand* tmp = new SCommand;
    tmp->name = name;
    tmp->pCommand = command;

    tmp->next = firstCmd;
    tmp = firstCmd;
}
