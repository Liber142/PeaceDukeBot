#pragma once

#include "command.h"

class CApplyCommand : public ICommand
{
public:
	CApplyCommand(CBotCore *pBoteCore);

	void OnInit() override {}
	void OnConsoleInit() override;

	void Execute(CConsole::IResult &Result) override;

	const std::string Name() const override { return "apply"; }
};
