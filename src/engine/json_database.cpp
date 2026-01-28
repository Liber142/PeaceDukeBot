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
