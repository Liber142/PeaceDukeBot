#pragma once

#include "command.h"

class CApply : public ICommand
{
public:
	CApply(CBotCore *pBoteCore);
    ~CApply() override = default;
	void Register() override;
	void Execute(const dpp::slashcommand_t &Event) override;
};
