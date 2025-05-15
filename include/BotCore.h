#pragma once
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <string>

class BotCore
{
private:
	dpp::cluster bot;
	void SetupEvent();
	void RegisterSlashCommands();
	void RegisterButton();
public:
	BotCore(std::string& token);
	void StartDataBase(std::string filePath);
	void Start();
};