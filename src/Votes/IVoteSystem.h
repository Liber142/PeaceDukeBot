#pragma once
#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>

class IVoteSystem
{
public:
    virtual ~IVoteSystem() = default;
    virtual void Initialize(dpp::cluster& bot) = 0;
    virtual void ProcessButtonClick(const dpp::button_click_t& event) = 0;
    virtual void ProcessFormSubmit(const dpp::form_submit_t& event) = 0;
    virtual void SaveState() = 0;
    virtual void LoadState() = 0;
};
