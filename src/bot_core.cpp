#include "bot_core.h"
#include "engine/config.h"

#include "modules/test.h"

CBotCore::CBotCore(dpp::cluster* pBot)
    : m_pBot(pBot)
{
    m_pConfig = new CConfig();
    m_pDataBase = new JsonDataBase();

    m_pDataBase->Connect("db");

    Test Test(this);
    Test.OnInit();
}
