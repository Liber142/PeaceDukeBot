#pragma once
#include <dpp/snowflake.h>

#include <dpp/nlohmann/json.hpp>
#include <string>

namespace nlohmann
{
	template<typename T>
	struct adl_serializer<std::optional<T>>
	{
		static void to_json(json &j, const std::optional<T> &opt)
		{
			if(opt)
				j = *opt;
			else
				j = nullptr;
		}
		static void from_json(const json &j, std::optional<T> &opt)
		{
			if(j.is_null())
				opt = std::nullopt;
			else
				opt = j.get<T>();
		}
	};
}

struct SBirthDate
{
	int m_Day = 0;
	int m_Month = 0;
	std::optional<int> m_Year;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SBirthDate, m_Day, m_Month, m_Year);
};

struct SUserData
{
	size_t m_Id = 0;
	SBirthDate m_BirthDay;
	std::optional<int> m_Age;
	int m_SocialRating = 0;
	std::string m_GameNick;
	std::string m_Clan;
	std::string m_About;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SUserData, m_Id, m_BirthDay, m_Age, m_SocialRating, m_GameNick, m_Clan, m_About)
};
