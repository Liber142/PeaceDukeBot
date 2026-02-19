#pragma once

#include <dpp/dpp.h>

#include <functional>
#include <string>
#include <vector>

//TODO: Change to enum class
enum : int
{
	SLASH_COMMAND = 1 << 1,
	BUTTON = 1 << 2,
	MODAL = 1 << 3,
	CFGFLAG_SAVE = 1 << 4,
};

class CConsole
{
public:
	class IResult
	{
	public:
		IResult(std::string &Name) :
			m_Name(Name) {}
		const std::string m_Name;
		std::vector<std::string> m_Args;

		int m_Flags = 0;
		const dpp::interaction_create_t *m_Event;

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

	//TODO: Add type system
	//We have m_vParams in CCommand and Params in Register()
	//This paramets say what is type in this command for future
	class CCommand
	{
	public:
		CCommand(std::string Name, FnCallBack Callback);
		const std::string m_Name;
		std::vector<std::string> m_vParams;
		int m_Flags = 0;
		FnCallBack m_CallBack;
		std::string m_Help;
	};
	CCommand *FindCommand(const std::string &Name, int Flags);
	CCommand *FindCommand(const std::string &Name);

	/**
	 * @brief Registers a console command
	 *
	 * Adds a new command to the console system. The command can be executed
	 * via chat, slash command, button, or modal form.
	 *
	 * @parameter std::string Name Command identifier
	 * @parameter std::vector<std::string>> Params List of parameter types
	 * @parameter int Flags Command flags
	 * @parameter FnCallback Callback Function to execute when command is called
	 * @parameter std::string Help Description shown in help commands
	 *
	 * @note If a command with same Name and Flags exists, registration is skipped
	 */
	void Register(const std::string &Name, const std::vector<std::string> &Params, int Flags, const FnCallBack &Callback, const std::string &Help);

	void ExecuteInteraction(const dpp::interaction_create_t &Event);
	void ExecuteLine(std::string &Line);
	void ExecuteFile(std::string &Path);

	std::vector<std::string> GetAllCommands();
private:
	std::vector<CConsole::IResult> ParseLine(const std::string &Line);
	std::vector<std::unique_ptr<CCommand>> m_vpCommands;
};
