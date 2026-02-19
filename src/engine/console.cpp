#include "console.h"

#include "logger.h"

#include <string>

CConsole::CCommand::CCommand(std::string Name, FnCallBack Callback) :
	m_Name(std::move(Name)),
	m_CallBack(std::move(Callback))
{
}

CConsole::CCommand *CConsole::FindCommand(const std::string &Name, int Flags)
{
	for(const auto &Cmd : m_vpCommands)
		if(Cmd->m_Flags & Flags && Cmd->m_Name == Name)
			return Cmd.get();

	return nullptr;
}

CConsole::CCommand *CConsole::FindCommand(const std::string &Name)
{
	for(const auto &Cmd : m_vpCommands)
		if(Cmd->m_Name == Name)
			return Cmd.get();

	return nullptr;
}

void CConsole::Register(const std::string &Name, const std::vector<std::string> &Params,
	int Flags, const FnCallBack &Callback, const std::string &Help)
{
	CConsole::CCommand *pCmd = FindCommand(Name, Flags);
	if(pCmd)
		return;

	std::unique_ptr<CConsole::CCommand> Cmd = std::make_unique<CConsole::CCommand>(Name, Callback);
	Cmd->m_vParams = Params;
	Cmd->m_Flags = Flags;
	Cmd->m_Help = Help;

	CLogger::Info("console", "Register command: " + Name);

	m_vpCommands.push_back(std::move(Cmd));
}

void CConsole::ExecuteInteraction(const dpp::interaction_create_t &Event)
{
	std::string Line;
	int InteractionFlag = 0;

	if(const auto *Button = dynamic_cast<const dpp::button_click_t *>(&Event))
	{
		Line = Button->custom_id;
		InteractionFlag = BUTTON;
	}
	else if(const auto *Form = dynamic_cast<const dpp::form_submit_t *>(&Event))
	{
		Line = Form->custom_id;
		for(const dpp::component &pComponent : Form->components)
		{
			std::string Params;
			for(char c : std::get<std::string>(pComponent.value))
			{
				if(c == '\\')
					Params += "\\\\";
				else if(c == '\"')
					Params += "\\\"";
				else
					Params += c;
			}
			Line += " \"" + Params + "\"";
		}
		InteractionFlag = MODAL;
	}
	else if(const auto *Command = dynamic_cast<const dpp::slashcommand_t *>(&Event))
	{
		Line = Command->command.get_command_name();
		InteractionFlag = SLASH_COMMAND;
	}

	auto Results = ParseLine(Line);
	for(auto &Result : Results)
	{
		if(CCommand *Cmd = FindCommand(Result.m_Name))
		{
			Result.m_Event = &Event;
			Result.m_Flags |= InteractionFlag;
			Cmd->m_CallBack(std::move(Result));
		}
	}
}

void CConsole::ExecuteLine(std::string &Line)
{
	std::vector<IResult> Results = ParseLine(Line);
	for(const IResult &Result : Results)
		if(CCommand *Cmd = FindCommand(Result.m_Name))
			Cmd->m_CallBack(Result);
		else
			CLogger::Error("console", "Command not found: " + Result.m_Name);
}

void CConsole::ExecuteFile(std::string &Path)
{
	std::ifstream File(Path);
	std::string Line;
	while(std::getline(File, Line))
		ExecuteLine(Line);
	File.close();
}

std::vector<std::string> CConsole::GetAllCommands()
{
	std::vector<std::string> Result;
	Result.reserve(m_vpCommands.size());
	for(const auto &Cmd : m_vpCommands)
	{
		Result.emplace_back(Cmd->m_Name);
	}
	return Result;
}

std::vector<CConsole::IResult> CConsole::ParseLine(const std::string &Line)
{
	std::vector<IResult> Results;
	auto It = Line.begin();

	if(Line.size() >= 3 && Line.substr(0, 3) == "mc;")
		It += 3;

	while(It != Line.end())
	{
		while(It != Line.end() && (std::isspace(*It) || *It == ';'))
			++It;
		if(It == Line.end() || *It == '#')
			break;

		std::vector<std::string> Tokens;
		bool InCommandBlock = true;

		while(InCommandBlock && It != Line.end())
		{
			while(It != Line.end() && std::isspace(*It))
				++It;
			if(It == Line.end() || *It == ';' || *It == '#')
			{
				if(It != Line.end() && *It == ';')
					InCommandBlock = false;
				break;
			}

			std::string Token;
			bool Quoted = false;
			while(It != Line.end())
			{
				if(*It == '\\')
				{
					if(++It != Line.end())
					{
						Token += *It;
					}
				}
				else if(*It == '"')
					Quoted = !Quoted;
				else if(!Quoted && (std::isspace(*It) || *It == ';' || *It == '#'))
					break;
				else
					Token += *It;
				++It;
			}
			if(!Token.empty())
				Tokens.push_back(std::move(Token));
		}

		if(!Tokens.empty())
		{
			IResult Res(Tokens[0]);
			for(size_t i = 1; i < Tokens.size(); ++i)
				Res.m_Args.push_back(std::move(Tokens[i]));

			Results.push_back(std::move(Res));
		}
	}
	return Results;
}
