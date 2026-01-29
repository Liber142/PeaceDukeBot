#include "apply.h"

CApplyCommand::CApplyCommand(CBotCore *pBotCore) :
	ICommand(pBotCore)
{
}

void CApplyCommand::OnConsoleInit()
{
	dpp::slashcommand Command = dpp::slashcommand(
		Name(),
		"Создаст сообщение с кнопкой для подачи заявки",
		BotCore()->Bot()->me.id);

	BotCore()->Bot()->global_command_create(Command);
	Console().Register(Name(), {}, SLASH_COMMAND, [this](CConsole::IResult Result) { Execute(Result); }, Command.description);
}

void CApplyCommand::Execute(CConsole::IResult &Result)
{
	if(!(Result.m_Flags & SLASH_COMMAND))
		return;

	try
	{
		dpp::message Msg("Чтобы подать заявку на вступление в клан - нажмите кнопку ниже и заполните все поля. Мы сделаем все возможное, чтобы обработать ее как можно быстрее!");

		Msg.add_component(
			dpp::component().add_component(
				dpp::component()
					.set_label("Подать заявку")
					.set_type(dpp::cot_button)
					.set_style(dpp::cos_primary)
					.set_id("apply_button")));

		CLogger::Debug("apply", "Hello");

		Result.m_Event.reply(Msg);
	}
	catch(std::exception &e)
	{
		CLogger::Error("apply", e.what());
	}
}
