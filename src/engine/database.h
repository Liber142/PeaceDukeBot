#pragma once

#include "data_strucs.h"

#include <dpp/snowflake.h>

class IDataBase
{
public:
	virtual ~IDataBase() = default;
	virtual void Connect(std::string Path) = 0;

	virtual SUserData ExtractUser(uint64_t Key) = 0;
	virtual void InsertUser(uint64_t Key, SUserData Data) = 0;

	virtual SVoteData ExtractVote(uint64_t Key) = 0;
	virtual void InsertVote(uint64_t Key, SVoteData Data) = 0;
};
