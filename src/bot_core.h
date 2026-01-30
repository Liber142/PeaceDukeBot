#pragma once
#include <engine/config.h>
#include <engine/console.h>
#include <engine/database.h>
#include <engine/logger.h>

#include <dpp/cluster.h>

#include <memory>

class IModule;

class CBotCore
{
public:
	CBotCore(dpp::cluster *pBot);
	virtual ~CBotCore();

	CConfig &Config()
	{
		return *m_pConfig;
	}
	const IDataBase &DataBase()
	{
		return *m_pDataBase;
	}
	CConsole &Console()
	{
		return *m_pConsole;
	}
	dpp::cluster *Bot()
	{
		return m_pBot;
	}

private:
	dpp::cluster *m_pBot;

	std::unique_ptr<CConfig> m_pConfig;
	std::unique_ptr<IDataBase> m_pDataBase;
	std::unique_ptr<CConsole> m_pConsole;

	std::vector<std::unique_ptr<IModule>> m_vpModules;

	void Init();
};
