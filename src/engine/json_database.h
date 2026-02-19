#pragma once
#include "database.h"

#include <condition_variable>
#include <queue>
#include <thread>

class CJsonDataBase : public IDataBase
{
public:
	~CJsonDataBase() override;
	void Connect(const std::string &Path, EDataBaseFlags Flags = None) override;

	void Erase(const std::string &Table, size_t Key) override;
	std::vector<size_t> GetKeys(const std::string &Table) override;
	size_t GenerateNewKey(const std::string &Table) override;

protected:
	void WriteRaw(const std::string &Table, const std::string &Key, const nlohmann::json &Data) override;
	nlohmann::json ReadRaw(const std::string &Table, const std::string &Key) override;

private:
	std::mutex m_Mutex;
	std::condition_variable m_CV;
	std::thread m_WorkerThread;
	std::atomic<bool> m_Running{true};
	std::atomic<bool> m_IsDirty{true};
	void WorkerLoop();

	std::string m_FilePath;
	EDataBaseFlags m_Flags;
	nlohmann::json m_Root;

	void Sync();
};
