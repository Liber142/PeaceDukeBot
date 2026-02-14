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
		std::string Color;
		std::string Prefix;
		std::ostream *Out = &std::cout;

		switch(Level)
		{
		case LogLevel::INFO: Color = "\033[0m"; Prefix = "[inf]"; break;
		case LogLevel::WARNING: Color = "\033[33m"; Prefix = "[wrn]"; break;
		case LogLevel::ERROR:
			Color = "\033[31m";
			Prefix = "[err]";
			Out = &std::cerr;
			break;
		case LogLevel::DEBUG: Color = "\033[36m"; Prefix = "[dbg]"; break;
		}

		*Out << Color << Timestamp << " " << Prefix << " " << From << ": " << Info << "\033[0m\n";
	}

	static void Info(const std::string &From, const std::string &Info) { Log(LogLevel::INFO, From, Info); }
	static void Error(const std::string &From, const std::string &Info) { Log(LogLevel::ERROR, From, Info); }
	static void Warning(const std::string &From, const std::string &Info) { Log(LogLevel::WARNING, From, Info); }
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
