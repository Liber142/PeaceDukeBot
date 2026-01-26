#pragma once

#include "command.h"

class CApply : public ICommand
{
public:
	CApply(CBotCore *pBoteCore);

	void OnInit() override {}

	void Register() override;
	void Execute(const dpp::slashcommand_t &Event) override;

	std::string Name() const override { return "apply"; }
};
