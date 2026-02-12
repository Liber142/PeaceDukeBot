#pragma once
#include "logger.h"

#include <dpp/nlohmann/json.hpp>
#include <string>

// TODO: This should be work on other threads
// I think we need queue for save data 
// Also now this is not threads safety
class IDataBase
{
public:
	virtual ~IDataBase() = default;
	virtual void Connect(const std::string &Path) = 0;

	template<typename T>
	void Save(std::string Table, uint64_t Key, const T &Data)
	{
		WriteRaw(Table, std::to_string(Key), Data);
	}

	template<typename T>
	T Load(const std::string &Table, uint64_t Key)
	{
		try
		{
			nlohmann::json J = ReadRaw(Table, std::to_string(Key));
			if(J.is_null() || J.empty())
				return T();
			return J.get<T>();
		}
		catch(const nlohmann::json::exception &e)
		{
			CLogger::Error("database", e.what());
			return T();
		}
	}

protected:
	virtual void WriteRaw(const std::string &Table, const std::string &Key, const nlohmann::json &Data) = 0;
	virtual nlohmann::json ReadRaw(const std::string &Table, const std::string &Key) = 0;
};
