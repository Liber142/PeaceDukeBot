#include "apply.h"

Apply::Apply(CBotCore* botcore)
    : ICommand(botcore)
{
    Register();
}

void Apply::Register()
{
    dpp::slashcommand command = dpp::slashcommand(
        "apply",
        "Создаст сообщение с кнопкой для подачи заявки",
        BotCore()->Bot()->me.id);

    BotCore()->Bot()->global_command_create(command);
}

void Apply::Execute()
{
}
