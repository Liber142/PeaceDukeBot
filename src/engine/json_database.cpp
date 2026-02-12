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

size_t CJsonDataBase::GenerateNewKey(const std::string &Table)
{
    nlohmann::json Meta = ReadRaw(Table, "0");
    
    size_t NextId = 1;
    if (!Meta.is_null() && Meta.contains("next_id")) {
        NextId = Meta["next_id"].get<size_t>();
    }

    Meta["next_id"] = NextId + 1;
    WriteRaw(Table, "0", Meta);

    return NextId;
}

std::vector<size_t> CJsonDataBase::GetKeys(const std::string &Table)
{
    std::vector<size_t> Keys;
    if (!m_Root.contains(Table) || !m_Root[Table].is_object())
        return Keys;

    for (const auto& [key, value] : m_Root[Table].items())
    {
        try 
        {
            size_t Id = std::stoull(key);
            if (Id == 0) 
                continue;
            Keys.emplace_back(Id);
        } 
        catch(...) 
        {
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
