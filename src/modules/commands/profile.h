#pragma once
#include "command.h"

class CProfileCommand : public ICommand
{
public:
	void OnInit() override;
	void OnConsoleInit() override;

	void Execute(CConsole::IResult &Result) override;

	const std::string Name() const override { return "profile"; }

private: 
	dpp::embed GenerateEmbend(const struct SUserData& Data);
	void PrintProfileConsole(const struct SUserData& Data);
	dpp::snowflake GetTargetUser(const dpp::slashcommand_t *Event);
};
