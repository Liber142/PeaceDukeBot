#pragma once

#include "engine/config.h"
#include "engine/database.h"

#include <dpp/cluster.h>

class IModule;

class CBotCore
{
public:
	CBotCore(dpp::cluster *pBot);

	~CBotCore();

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

private:
	dpp::cluster *m_pBot;

	CConfig *m_pConfig;
	IDataBase *m_pDataBase;

	struct SModule
	{
		IModule *m_pModule;
		SModule *m_pNext;
	} *m_pFirstModule = nullptr;

	void AddModule(IModule *pModule);

	void Init();
};
