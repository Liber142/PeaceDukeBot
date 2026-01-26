#pragma once

#include "../bot_core.h"

class IModule
{
public:
	IModule(CBotCore *pBotCore) :
		m_pBotCore(pBotCore)
	{
	}

	virtual ~IModule() = default;

	virtual void OnInit() = 0;

	virtual std::string Name() const { return "IModule"; }
	virtual CBotCore *BotCore()
	{
		return m_pBotCore;
	}

private:
	CBotCore *m_pBotCore;
};
