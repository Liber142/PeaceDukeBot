#pragma once
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <string>

#include "DataBase.h"
#include "CommandHandler.h"
#include "Online.h"

class BotCore
{
private:
    DataBase* v_db;
    DataBase* m_db;

	dpp::cluster bot;

	void SetupEvent();
	void RegisterSlashCommands();
	void RegisterButton();

	CommandHandler cmdHandler;
	C_OnlineClanMember OnlineClanMember;
public:
	BotCore(std::string& token);
	void StartDataBase(std::string v_filepath, std::string m_filepath, std::string AplicationAceptedMessage, std::string AplicationRejectedMessage);
	void Start();
};