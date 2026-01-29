#include "module.h"

class CClanManager : public IModule
{
public:
	CClanManager(CBotCore *pBotCore) :
		IModule(pBotCore) {}

	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CClanManager"; }
};
