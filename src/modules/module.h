#pragma once

#include "../bot_core.h"

class IModule
{
private:
	CBotCore *m_pBotCore;

public:
	IModule(CBotCore *pBotCore) :
		m_pBotCore(pBotCore)
	{
	}

    virtual ~IModule() = default;
	virtual void OnInit() = 0;
	virtual CBotCore *BotCore()
	{
		return m_pBotCore;
	}
};
