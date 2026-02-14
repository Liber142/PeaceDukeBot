#pragma once

#include "module.h"

class CMemberManager : public IModule
{
public:
	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CMemberManager"; }
};
