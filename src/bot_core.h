#pragma once

#include "engine/config.h"
#include "engine/database.h"

#include <dpp/cluster.h>

class CBotCore
{
private:
	dpp::cluster *m_pBot;

	CConfig *m_pConfig;
	IDataBase *m_pDataBase;

public:
	CBotCore(dpp::cluster *pBot);

	CConfig *Config()
	{
		return m_pConfig;
	}
	IDataBase *DataBase()
	{
		return m_pDataBase;
	}
	dpp::cluster *Bot()
	{
		return m_pBot;
	}
};
