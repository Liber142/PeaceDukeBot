#pragma once
#include <memory>
#include "IVoteSystem.h"

enum class EVoteSystemType
{
    ApplicationVote
};

class CVoteSystemFactory
{
public:
    static std::unique_ptr<IVoteSystem> CreateVoteSystem(EVoteSystemType type);
};
