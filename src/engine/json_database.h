#include "database.h"

class CJsonDataBase : public IDataBase
{
public:
	~CJsonDataBase() override = default;
	void Connect(std::string Path) override;
	SUserData GetUser(uint64_t Key) override;
	void AddUser(uint64_t Key, SUserData Data) override;

private:
	std::string m_FilePath;
	nlohmann::json m_Json;

	nlohmann::json toJson(SUserData Data);
	SUserData fromJson(nlohmann::json Data);

	void Update();
	void Save();
};
