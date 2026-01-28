#pragma once

#include "command.h"

class CApply : public ICommand
{
public:
	CApply(CBotCore *pBoteCore);

	void OnInit() override {}
    void OnConsoleInit() override;

	void Register() override;
	void Execute(CConsole::IResult& Result) override;

	std::string Name() const override { return "apply"; }
};
