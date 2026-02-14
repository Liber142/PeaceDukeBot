#include <bot_core.h>

void IModule::OnModuleInit(CBotCore *pBotCore)
{
	m_pBotCore = pBotCore;
}

class IDataBase *IModule::DataBase() const
{
	return m_pBotCore->DataBase();
}

class dpp::cluster *IModule::Bot() const
{
	return m_pBotCore->Bot();
}

class CConsole *IModule::Console() const
{
	return m_pBotCore->Console();
}

class CConfig *IModule::Config() const
{
	return m_pBotCore->Config();
}
