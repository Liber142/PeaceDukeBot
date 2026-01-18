#include "apply.h"

CApply::CApply(CBotCore* pBotCore)
    : ICommand(pBotCore)
{
    Register();
}

void CApply::Register()
{
    dpp::slashcommand Command = dpp::slashcommand(
        "apply",
        "Создаст сообщение с кнопкой для подачи заявки",
        BotCore()->Bot()->me.id);

    BotCore()->Bot()->global_command_create(Command);
}

void CApply::Execute()
{
}
