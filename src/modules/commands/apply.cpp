#include "apply.h"

#include <engine/console.h>
#include <engine/logger.h>

#include <bot_core.h>
#include <dpp/cluster.h>

void CApplyCommand::OnConsoleInit()
{
	if(!BotCore())
	{
		CLogger::Debug(Name(), "BotCoree is nullptr");
		return;
	}
	dpp::slashcommand Command = dpp::slashcommand(
		Name(),
		"Создаст сообщение с кнопкой для подачи заявки",
		Bot()->me.id);

	Bot()->global_command_create(Command);
	Console()->Register(Name(), {}, SLASH_COMMAND, [this](CConsole::IResult Result) { Execute(Result); }, Command.description);
	Console()->Register(Name() + "_button", {}, BUTTON, [this](CConsole::IResult Result) { Execute(Result); }, "Ответит модалкой на кнопку");
}

void CApplyCommand::Execute(CConsole::IResult &Result)
{
	if(Result.m_Flags & SLASH_COMMAND)
		Result.m_Event->reply(Message());
	else if(Result.m_Flags & BUTTON)
		Result.m_Event->dialog(Modal());
	else
		CLogger::Warning(Name(), "You can't use this from this");
}

dpp::message CApplyCommand::Message() const
{
	dpp::message Msg("Чтобы подать заявку на вступление в клан - нажмите кнопку ниже и заполните все поля. Мы сделаем все возможное, чтобы обработать ее как можно быстрее!");

	Msg.add_component(
		dpp::component().add_component(
			dpp::component()
				.set_label("Подать заявку")
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_primary)
				.set_id("apply_button")));

	return Msg;
}

dpp::interaction_modal_response CApplyCommand::Modal() const
{
	dpp::interaction_modal_response Modal(BotCore()->m_ClanVoteManager.Name() + "_form", "Заявка в клан");

	Modal.add_component(
		dpp::component()
			.set_label("Ваш игровой ник")
			.set_id("gamenick")
			.set_type(dpp::cot_text)
			.set_placeholder("Введите ваш никнейм")
			.set_max_length(32)
			.set_required(true)
			.set_text_style(dpp::text_short));

	Modal.add_row();
	Modal.add_component(
		dpp::component()
			.set_label("Дата Рождения (ДД.ММ или ДД.ММ.ГГГГ)")
			.set_id("birthday")
			.set_type(dpp::cot_text)
			.set_placeholder("Это чтобы поздравить тебя ")
			.set_required(true)
			.set_max_length(10)
			.set_min_length(5)
			.set_text_style(dpp::text_short));

	Modal.add_row();
	Modal.add_component(
		dpp::component()
			.set_label("Расскажите о себе")
			.set_id("about")
			.set_type(dpp::cot_text)
			.set_placeholder("Я очень добрый и общительный")
			.set_text_style(dpp::text_paragraph));

	return Modal;
}
