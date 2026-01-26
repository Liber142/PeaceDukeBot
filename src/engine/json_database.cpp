#include "json_database.h"

#include <fstream>
#include <iostream>

void CJsonDataBase::Connect(std::string Path)
{
	m_FilePath = Path + ".json";

	std::ifstream File(m_FilePath);
	if(!File.is_open())
	{
		std::ofstream NewFile(m_FilePath);
		NewFile << "{}";
		NewFile.close();
	}
	else
		File.close();
	Update();
}

SUserData CJsonDataBase::ExtractUser(uint64_t Key)
{
	SUserData Result;
	try
	{
		Update();
		Result = ConvertFromJson(m_Json["members"][Key], Result);
		return Result;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return Result;
}

SVoteData CJsonDataBase::ExtractVote(uint64_t Key)
{
	SVoteData Result;
	return Result;
}

void CJsonDataBase::InsertUser(uint64_t Key, SUserData Data)
{
	try
	{
		nlohmann::json j = ConvertToJson(Data);
		m_Json["members"][std::to_string(Key)] = j;
		Save();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void CJsonDataBase::InsertVote(uint64_t Key, SVoteData Data)
{
	try
	{
		nlohmann::json j = ConvertToJson(Data);
		m_Json["votes"][std::to_string(Key)] = j;
		Save();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void CJsonDataBase::Update()
{
	std::ifstream File(m_FilePath);
	if(!File.is_open())
	{
		std::cerr << "Failed open db file" << std::endl;
		return;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(File);
		File.close();
		m_Json = j;
	}
	catch(std::exception &e)
	{
		std::cerr << "Error parse data base: " << e.what() << std::endl;
	}
}

void CJsonDataBase::Save()
{
	std::ofstream File(m_FilePath);

	if(!File.is_open())
	{
		std::cerr << "Failed open db file" << std::endl;
		return;
	}

	try
	{
		File << m_Json.dump(4);
		File.close();
	}
	catch(std::exception &e)
	{
		std::cerr << "Error save data base file : " << e.what() << std::endl;
	}
}

nlohmann::json CJsonDataBase::ConvertToJson(const SUserData &Data)
{
	nlohmann::json Result = {
		{"about", Data.m_About},
		{"age", Data.m_Age},
		{"clan", Data.m_Clan},
		{"game_nick", Data.m_GameNick},
		{"social_rating", Data.m_SocialRating}};

	return Result;
}

nlohmann::json CJsonDataBase::ConvertToJson(const SVoteData &Data)
{
	nlohmann::json Result;

	return Result;
}

SUserData CJsonDataBase::ConvertFromJson(const nlohmann::json &Data, const SUserData &Type)
{
	SUserData Result;
	Result.m_Age = Data.value("age", 0);
	Result.m_SocialRating = Data.value("social_rating", 0);
	Result.m_GameNick = Data.value("game_nick", "");
	Result.m_Clan = Data.value("clan", "");
	Result.m_About = Data.value("about", "");
	return Result;
}

SVoteData CJsonDataBase::ConvertFromJson(const nlohmann::json &Data, const SVoteData &Type)
{
	SVoteData Result;
	return Result;
}
