#include "member_manager.h"

#include <engine/data_strucs.h>

class CClanMemberManager : public CMemberManager
{
public:
    CClanMemberManager(CBotCore *pBotCore) : CMemberManager(pBotCore) {}

    void OnInit() override;
    void OnConsoleInit() override;

    void AddClanMember(SUserData Member);
    void KickClanMember(dpp::snowflake Id);

	const std::string Name() const override { return "CClanMemberManager"; }
private:
    dpp::snowflake m_ClanGuildId;
};
