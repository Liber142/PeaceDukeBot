#pragma once
#include <string>
namespace replxx {struct Replxx {struct Completion{Completion(std::string){}};template<class F>void set_completion_callback(F){}const char* input(const char*){return nullptr;}void history_add(std::string){}};}
