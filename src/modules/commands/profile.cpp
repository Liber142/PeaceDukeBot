#include "profile.h"

#include <engine/config.h>
#include <engine/console.h>
#include <engine/data_strucs.h>
#include <engine/database.h>
#include <engine/logger.h>

#include <bot_core.h>
#include <dpp/cluster.h>
#include <dpp/dpp.h>

void CProfileCommand::OnInit()
{
	dpp::slashcommand Command = dpp::slashcommand(
		Name(),
		"Посмотри чей-либо профиль",
		Bot()->me.id);
	Command.add_option(dpp::command_option(dpp::co_user, "user", "chose user", false));

	Bot()->global_command_create(Command, [this](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			CLogger::Error(Name(), Callback.get_error().human_readable);
			return;
		}
	});
}

void CProfileCommand::OnConsoleInit()
{
	Console()->Register(Name(), {}, SLASH_COMMAND, [this](CConsole::IResult Result) { Execute(Result); }, "");
}

void CProfileCommand::Execute(CConsole::IResult &Result)
{
	if(Result.m_Flags & SLASH_COMMAND)
	{
		const auto *Event = dynamic_cast<const dpp::slashcommand_t *>(Result.m_Event);
		if(Event)
		{
			CLogger::Error(Name(), "Wrong result");
			Result.m_Event->reply("Ooops... error");
			return;
		}
		const auto &Param = Event->get_parameter("user");

		dpp::snowflake UserId;
		if(const auto *Ptr = std::get_if<dpp::snowflake>(&Param))
			UserId = std::get<dpp::snowflake>(Param);
		else
			UserId = Result.m_Event->command.get_issuing_user().id;

		if(UserId.empty())
		{
			Event->reply(dpp::message("User not found").set_flags(dpp::m_ephemeral));
			return;
		}

		const auto &Member = DataBase()->Load<SUserData>("clan_members", UserId);
		if(!Member.has_value())
		{
			Event->reply(dpp::message("User not found").set_flags(dpp::m_ephemeral));
			return;
		}

		dpp::embed Embed = GenerateEmbend(Member.value());
		Event->reply(dpp::message(Embed));
	}
	else
	{
		if(Result.NumArguments() == 0)
		{
			CLogger::Info(Name(), "Write user");
			return;
		}

		for(const auto &Arg : Result.m_Args)
		{
			size_t Id = 0;
			try
			{
				Id = std::stoll(Arg);
			}
			catch(...)
			{
				CLogger::Warning(Name(), "Wrong User");
				continue;
			}

			const auto &User = DataBase()->Load<SUserData>("clan_members", Id);
			if(!User.has_value())
			{
				CLogger::Warning(Name(), "User not found");
				continue;
			}

			PrintProfileConsole(User.value());
		}
	}
}

void CProfileCommand::PrintProfileConsole(const struct SUserData &Data)
{
	const std::string Bold = "\033[1m";
	const std::string Cyan = "\033[36m";
	const std::string Gray = "\033[90m";
	const std::string Reset = "\033[0m";

	auto PrintLine = [&](const std::string &Label, const std::string &Value) {
		std::cout << Gray << "│ " << Cyan << std::left << std::setw(8) << Label
			  << Reset << " " << Value << "\n";
	};

	std::cout << Gray << "┌── " << Bold << "PROFILE" << Reset << "\n";

	PrintLine("USER:", Data.m_GameNick);

	if(!Data.m_About.empty())
	{
		PrintLine("ABOUT:", Data.m_About);
	}

	PrintLine("CLAN:", Data.m_Clan.empty() ? "None" : Data.m_Clan);

	std::stringstream BirthDateStream;
	BirthDateStream << std::setw(2) << std::setfill('0') << Data.m_BirthDay.m_Day << "."
			<< std::setw(2) << std::setfill('0') << Data.m_BirthDay.m_Month;
	PrintLine("B-DAY:", BirthDateStream.str());

	if(Data.m_Age.has_value())
	{
		PrintLine("AGE:", std::to_string(Data.m_Age.value()));
	}

	std::cout << Gray << "└───────────────────────────" << Reset << "\n";
}

dpp::embed CProfileCommand::GenerateEmbend(const SUserData &Data)
{
	std::stringstream BirthDateStream;
	BirthDateStream << std::setw(2) << std::setfill('0') << Data.m_BirthDay.m_Day << "."
			<< std::setw(2) << std::setfill('0') << Data.m_BirthDay.m_Month;

	dpp::embed Embed = dpp::embed()
				   .set_color(0x5865F2)
				   .set_title("👤 " + Data.m_GameNick)
				   .set_description(Data.m_About.empty() ? "_Информация не указана_" : Data.m_About);

	if(!Data.m_Clan.empty())
	{
		Embed.add_field("🛡️ Клан", Data.m_Clan, true);
	}

	Embed.add_field("📅 Дата Рождения", BirthDateStream.str(), true);

	if(Data.m_Age.has_value())
	{
		Embed.add_field("🎂 Возраст", std::to_string(Data.m_Age.value()), true);
	}

	return Embed;
}

dpp::snowflake CProfileCommand::GetTargetUser(const dpp::slashcommand_t *Event)
{
	dpp::snowflake Param = std::get<dpp::snowflake>(Event->get_parameter("user"));

	if(Param.empty())
		return Event->command.get_issuing_user().id;
	else
		return Param;
}
