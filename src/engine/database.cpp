#include "database.h"

#include <fstream>
#include <iostream>

void SJsonDataBase::Connect(std::string Path)
{
	m_FilePath = Path + ".json";

	std::ifstream file(m_FilePath);
	if(!file.is_open())
	{
		std::ofstream nfile(m_FilePath);
		nfile << "{}";
		nfile.close();
	}
	else
		file.close();
	Update();
}

SUserData CJsonDataBase::GetUser(uint64_t Key)
{
	SUserData result;
	try
	{
		result = fromJson(m_Json[Key]);
		result.m_Id = Key;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return result;
}

void CJsonDataBase::AddUser(uint64_t key, SUserData data)
{
	try
	{
		nlohmann::json j = toJson(data);
		m_Json["members"][std::to_string(key)] = j;
		Save();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void CJsonDataBase::Update()
{
	std::ifstream file(m_FilePath);
	if(!file.is_open())
	{
		std::cerr << "Failed open db file" << std::endl;
		return;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(file);
		file.close();
		m_Json = j;
	}
	catch(std::exception &e)
	{
		std::cerr << "Error parse data base: " << e.what() << std::endl;
	}
}

void CJsonDataBase::Save()
{
	std::ofstream file(m_FilePath);

	if(!file.is_open())
	{
		std::cerr << "Failed open db file" << std::endl;
		return;
	}

	try
	{
		file << m_Json.dump(4);
		file.close();
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
