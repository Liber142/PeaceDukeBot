#include <dpp/dpp.h>
#include <dpp/message.h>

class C_OnlineClanMember
{
private:
    struct MsgData
    {
        std::string ip;
        std::string connectUrl;
        std::string serverName;
        std::string mapName;

        std::vector<std::string> clientName;
        bool afk;
    };
    nlohmann::json data;
    nlohmann::json lastData;

    void ParsData(nlohmann::json data);
    dpp::message CreateMsg();

    std::vector<MsgData> Servers;

    dpp::timer update_timer;
    dpp::snowflake last_message_id = 0;

    void UpdateMessage(dpp::cluster& bot);
    void CreateNewMessage(dpp::cluster& bot);

public:
    void Init(dpp::cluster& bot);
};
