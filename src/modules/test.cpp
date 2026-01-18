#include "test.h"

#include <iostream>

void Test::OnInit()
{
	std::cout << BotCore()->Config()->about << std::endl;
}
