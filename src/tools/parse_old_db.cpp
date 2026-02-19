#include <engine/data_strucs.h>
#include <engine/json_database.h>
#include <engine/logger.h>

struct SOldUserData
{
	std::string about;
	std::string age;
	std::string game_nick;
	std::string clan;
	int social_rating;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SOldUserData, about, age, game_nick, clan, social_rating);
};

std::string Name() { return "parse_old_db"; }

int main(int args, char *argv[])
{
	if(args <= 1)
	{
		CLogger::Error(Name(), "Missing any files");
		return 1;
	}

	std::string Input = std::string(argv[1]);
	std::optional<std::string> Output = std::nullopt;
	if(args > 2)
		Output = std::string(argv[2]);

	CJsonDataBase DataBase;
	DataBase.Connect(Output.value_or("db.json"), EDataBaseFlags::ReadOnly);

	CJsonDataBase OldDataBase;
	OldDataBase.Connect(Input, EDataBaseFlags::ReadOnly);

	std::vector<size_t> Keys = OldDataBase.GetKeys("");
	CLogger::Info(Name(), "Found keys: " + std::to_string(Keys.size()));
	for(const size_t &Key : Keys)
	{
		auto OldUser = OldDataBase.Load<SOldUserData>("", Key);
		if(!OldUser.has_value())
			continue;

		CLogger::Info(Name(), "Found user: " + OldUser->game_nick);

		SUserData User;
		User.m_Id = Key;
		User.m_About = OldUser->about;
		User.m_Age = std::stoi(OldUser->age);
		User.m_GameNick = OldUser->game_nick;
		User.m_Clan = OldUser->clan;
		User.m_SocialRating = OldUser->social_rating;

		DataBase.Save("clan_members", Key, User);
	}
}
