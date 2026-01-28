#include "console.h"

#include "logger.h"

#include <string>

CConsole::CCommand::CCommand(const std::string &Name, FnCallBack &Callback) :
	m_Name(Name),
	m_CallBack(Callback)
{
}

CConsole::CCommand *CConsole::FindCommand(const std::string &Name, int Flags)
{
	for(CCommand &Cmd : m_vCommands)
	{
		if(Cmd.m_Flags & Flags)
		{
			if(Cmd.m_Name == Name)
			{
				return &Cmd;
			}
		}
	}
	return nullptr;
}

void CConsole::Register(const std::string &Name, const std::vector<std::string> &Params,
	int Flags, FnCallBack Callback, const std::string &Help)
{
	CConsole::CCommand *pCmd = FindCommand(Name, Flags);
	if(pCmd)
		return;

	CConsole::CCommand Cmd(Name, Callback);
	Cmd.m_Flags = Flags;
	Cmd.m_Flags = Flags;
	Cmd.m_Help = Help;

	CLogger::Info("console", "Register " + Name + " command");

	m_vCommands.emplace_back(std::move(Cmd));
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
		pCommand->m_CallBack(Result);
		CLogger::Info("console", "Execute " + Name + " command");
	}
}

void CConsole::ExecuteLine(std::string &Line)
{
}

void CConsole::ExecuteFile(std::string &Path)
{
}
