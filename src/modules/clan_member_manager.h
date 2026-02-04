#pragma once
#include "member_manager.h"

#include <engine/data_strucs.h>

class CClanMemberManager : public CMemberManager
{
public:
    void OnInit() override;
    void OnConsoleInit() override;

	const std::string Name() const override { return "CClanMemberManager"; }
private:

    void AddClanMember(SUserData Member);
    void KickClanMember(dpp::snowflake Id);

    dpp::snowflake m_ClanGuildId;
};
