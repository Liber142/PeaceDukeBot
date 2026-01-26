#include "apply.h"

CApply::CApply(CBotCore *pBotCore) :
	ICommand(pBotCore)
{
}

void CApply::Register()
{
	dpp::slashcommand Command = dpp::slashcommand(
		Name(),
		"Создаст сообщение с кнопкой для подачи заявки",
		BotCore()->Bot()->me.id);

	BotCore()->Bot()->global_command_create(Command);
}

void CApply::Execute(const dpp::slashcommand_t &Event)
{
	try
	{
		dpp::message Msg(
			Event.command.channel_id,
			"Чтобы подать заявку на вступление в клан - нажмите кнопку ниже и заполните все поля. Мы сделаем все возможное, чтобы обработать ее как можно быстрее!");

		Msg.add_component(
			dpp::component().add_component(
				dpp::component()
					.set_label("Подать заявку")
					.set_type(dpp::cot_button)
					.set_style(dpp::cos_primary)
					.set_id("apply_button")));

		Event.reply(Msg);
	}
	catch(std::exception &e)
	{
		CLogger::Error("apply", e.what());
	}
}
