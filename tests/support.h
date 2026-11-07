#pragma once
#include <doctest/doctest.h>
#include <dpp/dpp.h>
#include <engine/data_strucs.h>
#include <engine/json_database.h>
#include <engine/config.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <set>
namespace fs=std::filesystem;
struct Sandbox {
 fs::path previous=fs::current_path(),root;
 Sandbox(){std::string pattern=(fs::temp_directory_path()/"peace-tests-XXXXXX").string();std::vector<char> b(pattern.begin(),pattern.end());b.push_back(0);auto p=mkdtemp(b.data());if(!p)throw std::runtime_error("mkdtemp failed");root=p;fs::current_path(root);fs::create_directory("data");}
 ~Sandbox(){fs::current_path(previous);fs::remove_all(root);}
};
inline void write_file(std::string path,std::string text){std::ofstream f(path);f<<text;}
inline std::string read_file(std::string path){std::ifstream f(path);return {std::istreambuf_iterator<char>(f),{}};}
inline void line(CConsole& c,std::string s){c.ExecuteLine(s);}
struct Capture {std::ostringstream out,err;std::streambuf *o=std::cout.rdbuf(out.rdbuf()),*e=std::cerr.rdbuf(err.rdbuf());~Capture(){std::cout.rdbuf(o);std::cerr.rdbuf(e);}};
inline SUserData sample(){SUserData u;u.m_Id=42;u.m_GameNick="Игрок 🎮";u.m_About="quote \" and \\ newline\n";u.m_Clan="Peace Duke";u.m_Age=25;u.m_BirthDay={29,2,2000};u.m_SocialRating=-7;return u;}
