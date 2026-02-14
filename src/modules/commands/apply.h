#pragma once

#include "command.h"

class CApplyCommand : public ICommand
{
public:
	void OnInit() override {}
	void OnConsoleInit() override;

	void Execute(CConsole::IResult &Result) override;

	const std::string Name() const override { return "apply"; }

private:
	dpp::message Message() const;
	dpp::interaction_modal_response Modal() const;
};
