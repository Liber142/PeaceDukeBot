#include "database.h"
#include "dpp/nlohmann/json.hpp"

class CJsonDataBase : public IDataBase
{
public:
	~CJsonDataBase() override = default;
	void Connect(std::string Path) override;

	SUserData ExtractUser(uint64_t Key) override;
	void InsertUser(uint64_t Key, SUserData Data) override;

	SVoteData ExtractVote(uint64_t Key) override;
	void InsertVote(uint64_t Key, SVoteData Data) override;

private:
	std::string m_FilePath;
	nlohmann::json m_Json;

	nlohmann::json ConvertToJson(const SUserData &Data);
	nlohmann::json ConvertToJson(const SVoteData &Data);
	SUserData ConvertFromJson(const nlohmann::json &Data, const SUserData &Type);
	SVoteData ConvertFromJson(const nlohmann::json &Data, const SVoteData &Type);

	void Update();
	void Save();
};
