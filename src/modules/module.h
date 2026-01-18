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

	virtual void OnInit()
	{
	}
	virtual CBotCore *BotCore()
	{
		return m_pBotCore;
	}
};
