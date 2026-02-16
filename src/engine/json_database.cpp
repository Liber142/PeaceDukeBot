#include "json_database.h"

#include "logger.h"

#include <fstream>

CJsonDataBase::~CJsonDataBase()
{
	Sync();
}

void CJsonDataBase::Connect(const std::string &Path)
{
	m_FilePath = Path + ".json";
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
}

void CJsonDataBase::WriteRaw(const std::string &Table, const std::string &Key, const nlohmann::json &Data)
{
	m_Root[Table][Key] = Data;
	Sync();
}

nlohmann::json CJsonDataBase::ReadRaw(const std::string &Table, const std::string &Key)
{
	if(m_Root.contains(Table) && m_Root[Table].contains(Key))
	{
		return m_Root[Table][Key];
	}
	return nullptr;
}

void CJsonDataBase::Erase(const std::string &Table, size_t Key)
{
	if(m_Root.contains(Table))
	{
		m_Root[Table].erase(std::to_string(Key));
		Sync();
	}
}

size_t CJsonDataBase::GenerateNewKey(const std::string &Table)
{
	nlohmann::json Meta = ReadRaw(Table, "_meta");

	size_t NextId = 1;
	if(!Meta.is_null() && Meta.contains("next_id"))
	{
		NextId = Meta["next_id"].get<size_t>();
	}

	Meta["next_id"] = NextId + 1;
	WriteRaw(Table, "_meta", Meta);

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

void CJsonDataBase::Sync()
{
	std::ofstream File(m_FilePath);
	if(File.is_open())
	{
		File << m_Root.dump(4);
		File.close();
	}
	else
	{
		CLogger::Error("Database", "Failed to sync database to file: " + m_FilePath);
	}
}
