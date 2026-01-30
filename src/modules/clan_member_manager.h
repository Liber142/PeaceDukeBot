#include "member_manager.h"

class CClanMemberManager : public CMemberManager
{
public:
    CClanMemberManager(CBotCore *pBotCore) : CMemberManager(pBotCore) {}

    void OnInit() override;
    void OnConsoleInit() override;

	const std::string Name() const override { return "CClanMemberManager"; }
};
