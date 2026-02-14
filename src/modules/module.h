#pragma once
#include <dpp/dpp.h>

#include <string>

class IModule
{
public:
	virtual void OnModuleInit(class CBotCore *pBotCore);
	virtual ~IModule() = default;

	virtual void OnInit() = 0;
	virtual void OnConsoleInit() = 0;

	virtual const std::string Name() const { return "IModule"; }

protected:
	class CBotCore *BotCore() const { return m_pBotCore; }

	class IDataBase *DataBase() const;

	class CConsole *Console() const;

	class CConfig *Config() const;

	class dpp::cluster *Bot() const;

private:
	class CBotCore *m_pBotCore = nullptr;
};
