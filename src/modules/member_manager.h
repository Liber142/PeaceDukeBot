#pragma once 

#include "module.h"

class CMemberManager : public IModule
{
public:
	CMemberManager(CBotCore *pBotCore) :
		IModule(pBotCore) {}

	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CMemberManager"; }
protected:
    dpp::cluster* Bot() { return BotCore()->Bot(); }
};
