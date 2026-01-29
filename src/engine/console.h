#pragma once

#include <dpp/dpp.h>

#include <functional>
#include <string>
#include <vector>

enum : int
{
	SLASH_COMMAND = 1 << 1,
	CFGFLAG_SAVE = 1 << 2,
};

class CConsole
{
public:
	class IResult
	{
	public:
		IResult(std::string &Name) :
			m_Name(Name) {}
		std::string &m_Name;
		std::vector<std::string> m_Args;

		int m_Flags;
		dpp::interaction_create_t m_Event;

		int NumArguments() const { return m_Args.size(); }
		std::string GetString(int Index) const { return m_Args[Index]; }
		int GetInt(int Index) const
		{
			try
			{
				return std::stoi(m_Args[Index]);
			}
			catch(...)
			{
				return -1;
			};
		}
	};

	using FnCallBack = std::function<void(IResult Result)>;

	class CCommand
	{
	public:
		CCommand(std::string Name, FnCallBack Callback);
		const std::string m_Name;
		std::vector<std::string> m_vParams;
		int m_Flags;
		FnCallBack m_CallBack;
		std::string m_Help;
	};
	CCommand *FindCommand(const std::string &Name, int Flags);
	CCommand *FindCommand(const std::string &Name);

	/* std::string& Name
	 * std::vector<std::string>& Params
	 * int& Flags
	 * FnCallBack Callback
	 * std::string& Help
	 */
	void Register(const std::string &Name, const std::vector<std::string> &Params, int Flags, FnCallBack Callback, const std::string &Help);

	void ExecuteInteraction(const dpp::interaction_create_t &Event);
	void ExecuteLine(std::string &Line);
	void ExecuteFile(std::string &Path);

private:
	std::vector<std::unique_ptr<CCommand>> m_vpCommands;
};
