#pragma once

#include "module.h"

class CMemberManager : public IModule
{
public:
	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CMemberManager"; }

	int NumModers() const { return m_NumModers; }
	bool IsModerator(const dpp::snowflake &UserId);

private:
	int m_NumModers = 0;
	std::vector<dpp::snowflake> m_vModers;

	void RegisterEvents();
	bool IsModerator(const std::vector<dpp::snowflake> &Roles);
	void HandleMemberAdd(const dpp::guild_member_add_t &Event);
	void HandleMemberUpdate(const dpp::guild_member_update_t &Event);
	void SyncModerators(const dpp::confirmation_callback_t &Callback);
};
