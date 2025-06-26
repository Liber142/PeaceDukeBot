#pragma once

#include <dpp/snowflake.h>
#include <string>
constexpr dpp::snowflake CHANNEL_MODERATION_ID = 1271535210537680897; // ID канала модерации
constexpr dpp::snowflake CLAN_ROLE_ID = 987654321098765432; // ID роли клана
constexpr dpp::snowflake DEFAULT_ROLE_ID = 1252679487968120852;
constexpr dpp::snowflake GENERAL_ROLE_ID = 1298109770355576946;

constexpr dpp::snowflake CHANNEL_ONLINE_MEMBERS_ID = 1387611490571522241;
constexpr dpp::snowflake CHANNEL_BOT_CLIENT_ID = 1372534646515957842;
constexpr dpp::snowflake EVENT_PANEL_CHANNEL_ID = 1382514790160666794;

const std::string PATH_MEMBERS_DATA_BASE = "members_data.json";
const std::string PATH_VOTES_DATA_BASE = "active_votes.json";
const std::string PATH_CONFIG = "config.json";