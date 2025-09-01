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

public:
    void Init(dpp::cluster& bot);
};
