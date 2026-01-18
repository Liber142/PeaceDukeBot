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

SUserData CJsonDataBase::GetUser(uint64_t Key)
{
	SUserData Result;
	try
	{
		Result = fromJson(m_Json[Key]);
		Result.m_Id = Key;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return Result;
}

void CJsonDataBase::AddUser(uint64_t Key, SUserData Data)
{
	try
	{
		nlohmann::json j = toJson(Data);
		m_Json["members"][std::to_string(Key)] = j;
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

nlohmann::json CJsonDataBase::toJson(SUserData Data)
{
	nlohmann::json Result;

	Result = {
		{"about", Data.m_About},
		{"age", Data.m_Age},
		{"clan", Data.m_Clan},
		{"game_nick", Data.m_GameNick},
		{"social_rating", Data.m_SocialRating}};

	return Result;
}

SUserData CJsonDataBase::fromJson(nlohmann::json Data)
{
	SUserData Result;

	Result.m_Age = Data.value("age", 0);
	Result.m_SocialRating = Data.value("social_rating", 0);
	Result.m_GameNick = Data.value("game_nick", "");
	Result.m_Clan = Data.value("clan", "");
	Result.m_About = Data.value("about", "");

	return Result;
}
