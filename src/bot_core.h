#pragma once
#include <dpp/cluster.h>

//Modules
#include "modules/clan_member_manager.h"
#include "modules/command_handler.h"
#include "modules/votes.h"

#include <memory>

class CConfig;
class IDataBase;
class CConsole;
class IModule;

class CBotCore
{
private:
	dpp::cluster *m_pBot;

	std::unique_ptr<class CConfig> m_pConfig;
	std::unique_ptr<class IDataBase> m_pDataBase;
	std::unique_ptr<class CConsole> m_pConsole;

	void Init();

public:
	CBotCore(dpp::cluster *pBot);
	virtual ~CBotCore();

	class CConfig *Config() const;
	class IDataBase *DataBase() const;
	class CConsole *Console() const;
	dpp::cluster *Bot() const;

public:
	//All modules
	CCommandHandler m_CommandHandler;
	CClanMemberManager m_ClanMemberManager;
	CApplyVoteManager m_ClanVoteManager;

private:
	std::vector<class IModule *> m_vpModules;
};
