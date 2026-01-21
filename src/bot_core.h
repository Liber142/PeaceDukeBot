#pragma once
#include <memory>

#include "engine/config.h"
#include "engine/database.h"

#include <dpp/cluster.h>

class IModule;

class CBotCore
{
public:
	CBotCore(dpp::cluster *pBot);

	~CBotCore();

	CConfig& Config()
	{
		return *m_pConfig;
	}
	const IDataBase& DataBase()
	{
		return *m_pDataBase;
	}
	const dpp::cluster& Bot()
	{
		return *m_pBot;
	}

private:
    dpp::cluster *m_pBot;

    std::unique_ptr<CConfig> m_pConfig;
    std::shared_ptr<IDataBase> m_pDataBase;

    std::vector<std::unique_ptr<IModule>> m_vpModules;

	void Init();
};
