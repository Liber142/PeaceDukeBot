#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel
{
	INFO,
	WARNING,
	ERROR,
	DEBUG
};

class CLogger
{
public:
	static void Log(LogLevel Level, const std::string &From, const std::string &Info)
	{
		std::string Timestamp = GetTimestamp();
		std::string Prefix;
		std::ostream *Out = &std::cout;

		switch(Level)
		{
		case LogLevel::INFO: Prefix = "[\033[32mINFO\033[0m]"; break; // Зеленый
		case LogLevel::WARNING: Prefix = "[\033[33mWARN\033[0m]"; break; // Желтый
		case LogLevel::ERROR:
			Prefix = "[\033[31mFAIL\033[0m]";
			Out = &std::cerr;
			break; // Красный
		case LogLevel::DEBUG: Prefix = "[\033[36mDEBG\033[0m]"; break; // Голубой
		}

		*Out << Timestamp << " " << Prefix << " [" << From << "] " << Info << std::endl;
	}

	static void Info(const std::string &From, const std::string &Info) { Log(LogLevel::INFO, From, Info); }
	static void Error(const std::string &From, const std::string &Info) { Log(LogLevel::ERROR, From, Info); }
	static void Debug(const std::string &From, const std::string &Info) { Log(LogLevel::DEBUG, From, Info); }

private:
	static std::string GetTimestamp()
	{
		auto Now = std::chrono::system_clock::now();
		std::time_t NowC = std::chrono::system_clock::to_time_t(Now);
		std::tm NowTm = *std::localtime(&NowC);

		std::ostringstream Oss;
		Oss << std::put_time(&NowTm, "%Y-%m-%d %H:%M:%S");
		return Oss.str();
	}
};
