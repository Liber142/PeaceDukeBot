#pragma once

#include "command.h"

class CApply : public ICommand
{
public:
	CApply(CBotCore *pBoteCore);

	void OnInit() override {}
	void OnConsoleInit() override;

	void Execute(CConsole::IResult &Result) override;

	const std::string Name() const override { return "apply"; }
};
