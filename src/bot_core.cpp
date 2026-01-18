#include "bot_core.h"
#include "engine/config.h"

#include "modules/test.h"

CBotCore::CBotCore(dpp::cluster* bot)
    : bot(bot)
{
    config = new CConfig();
    db = new JsonDataBase();

    db->Connect("db");

    Test test(this);
    test.OnInit();
}
