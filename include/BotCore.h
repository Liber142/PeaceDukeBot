#pragma once
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <string>
#include "DataBase.h"

class BotCore
{
private:
    DataBase* v_db;
    DataBase* m_db;

	dpp::cluster bot;
	void SetupEvent();
	void RegisterSlashCommands();
	void RegisterButton();
public:
	BotCore(std::string& token);
	void StartDataBase(std::string v_filePath, std::string m_filePath);
	void Start();
};