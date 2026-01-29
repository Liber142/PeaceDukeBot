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
	int Flags, FnCallBack Callback, const std::string &Help)
{
	CConsole::CCommand *pCmd = FindCommand(Name, Flags);
	if(pCmd)
		return;

	std::unique_ptr<CConsole::CCommand> Cmd = std::make_unique<CConsole::CCommand>(Name, Callback);
	Cmd->m_vParams = Params;
	Cmd->m_Flags = Flags;
	Cmd->m_Help = Help;

	CLogger::Info("console", "Register " + Name + " command");

	m_vpCommands.push_back(std::move(Cmd));
}

void CConsole::ExecuteSlash(const dpp::slashcommand_t &Event)
{
	std::string Name = Event.command.get_command_name();
	CConsole::CCommand *pCommand = FindCommand(Name, SLASH_COMMAND);
	if(pCommand)
	{
		CConsole::IResult Result(Name);
		Result.m_Event = Event;
		Result.m_Flags = pCommand->m_Flags;
		CLogger::Info("console", "Execute " + Name + " command");
		pCommand->m_CallBack(std::move(Result));
	}
}

void CConsole::ExecuteLine(std::string &Line)
{
	std::vector<IResult> Results;
	auto It = Line.begin();

	if(Line.substr(0, 3) == "mc;")
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
				if(*It == '"')
				{
					Quoted = !Quoted;
				}
				else if(*It == '\\' && (It + 1) != Line.end() && *(It + 1) == '"')
				{
					Token += '"';
					++It;
				}
				else if(!Quoted && (std::isspace(*It) || *It == ';' || *It == '#'))
				{
					break;
				}
				else
				{
					Token += *It;
				}
				++It;
			}
			if(!Token.empty())
				Tokens.push_back(std::move(Token));
		}

		if(!Tokens.empty())
		{
			IResult Result(Tokens[0]);
			for(size_t i = 1; i < Tokens.size(); ++i)
				Result.m_Args.push_back(std::move(Tokens[i]));

			CCommand *Cmd = FindCommand(Tokens[0]);
			if(!Cmd)
			{
				CLogger::Error("console", "can't find command: " + Tokens[0]);
				break;
			}

			Cmd->m_CallBack(std::move(Result));
		}
	}
}

void CConsole::ExecuteFile(std::string &Path)
{
	std::ifstream File(Path);
	std::string Line;
	while(std::getline(File, Line))
		ExecuteLine(Line);
	File.close();
}
