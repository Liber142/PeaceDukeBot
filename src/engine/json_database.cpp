#include "json_database.h"

#include "logger.h"

#include <fstream>
#include <mutex>

CJsonDataBase::~CJsonDataBase()
{
	m_Running = false;
	m_CV.notify_all();
	m_WorkerThread.join();
}

void CJsonDataBase::Connect(const std::string &Path, EDataBaseFlags Flags)
{
	m_Flags = Flags;
	m_FilePath = Path;
	if(Path.find(".json") == std::string::npos)
		m_FilePath += ".json";

	std::ifstream File(m_FilePath);

	if(File.is_open())
	{
		try
		{
			File >> m_Root;
		}
		catch(...)
		{
			m_Root = nlohmann::json::object();
		}
		File.close();
	}
	else
	{
		m_Root = nlohmann::json::object();
		Sync();
	}

	m_WorkerThread = std::thread(&CJsonDataBase::WorkerLoop, this);
}

void CJsonDataBase::WriteRaw(const std::string &Table, const std::string &Key, const nlohmann::json &Data)
{
	if(m_Flags & ReadOnly)
	{
		CLogger::Error("database", "Can't write to database: ReadOnly mode");
		throw std::logic_error("Database is opened in Read-Only mode");
	}
	{
		std::lock_guard<std::mutex> Lock(m_Mutex);
		m_Root[Table][Key] = Data;
	}
	m_IsDirty = true;
	m_CV.notify_one();
}

nlohmann::json CJsonDataBase::ReadRaw(const std::string &Table, const std::string &Key)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	if(m_Root.contains(Table) && m_Root[Table].contains(Key))
	{
		return m_Root[Table][Key];
	}
	return nullptr;
}

void CJsonDataBase::Erase(const std::string &Table, size_t Key)
{
	if(m_Flags & ReadOnly)
	{
		CLogger::Error("database", "Can't erase from database: ReadOnly mode");
		throw std::logic_error("Database is opened in Read-Only mode");
	}

	if(m_Root.contains(Table))
	{
		std::lock_guard<std::mutex> Lock(m_Mutex);
		m_Root[Table].erase(std::to_string(Key));
	}
	m_IsDirty = true;
}

size_t CJsonDataBase::GenerateNewKey(const std::string &Table)
{
	int NextId = 1;
	{
		std::lock_guard<std::mutex> Lock(m_Mutex);
		if (m_Root.contains(Table) && m_Root[Table].contains("_meta")) 
		{
			auto& Meta = m_Root[Table]["_meta"];
			if (Meta.contains("next_id")) {
				NextId = Meta["next_id"].get<size_t>();
			}
		}
		m_Root[Table]["_meta"]["next_id"] = NextId + 1;
	}
	m_IsDirty = true;
	return NextId;
}

std::vector<size_t> CJsonDataBase::GetKeys(const std::string &Table)
{
	std::vector<size_t> Keys;
	if(!m_Root.contains(Table) || !m_Root[Table].is_object())
		return Keys;

	for(const auto &[Key, Value] : m_Root[Table].items())
	{
		try
		{
			if(Key == "_meta")
				continue;
			size_t Id = std::stoull(Key);
			Keys.emplace_back(Id);
		}
		catch(const std::exception &e)
		{
			CLogger::Error("database", e.what());
			continue;
		}
	}
	return Keys;
}

void CJsonDataBase::WorkerLoop()
{
	while (m_Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (m_IsDirty) 
		{
            Sync();
            m_IsDirty = false;
        }
    }
}

void CJsonDataBase::Sync()
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	std::ofstream File(m_FilePath);
	if(File.is_open())
	{
		File << m_Root.dump();
		File.close();
	}
	else
	{
		CLogger::Error("Database", "Failed to sync database to file: " + m_FilePath);
	}
}
