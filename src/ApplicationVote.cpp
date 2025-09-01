#include <fmt/format.h>
#include <algorithm>
#include "ApplicationVote.h"
#include "ConstAgr.h"
#include "Parsing.h"

void CApplicationVoteSystem::Initialize(dpp::cluster& bot)
{
    LoadState();

    bot.on_button_click([this, &bot](const dpp::button_click_t& event)
                        {
        if (event.custom_id == "accept" || event.custom_id == "reject") 
        {
            ProcessButtonClick(event);
        } });
}

void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    auto it = m_activeVotes.find(event.command.message_id);
    if (it == m_activeVotes.end())
    {
        event.reply(dpp::message("Голосование не найдено или завершено").set_flags(dpp::m_ephemeral));
        return;
    }

    SApplicationVoteData& vote = it->second;
    dpp::user user = event.command.get_issuing_user();

    if (!vote.m_votedUsers.count(user.id))
    {
        (event.custom_id == "accept") ? vote.m_voteAccept++ : vote.m_voteReject++;
        vote.m_votedUsers.insert(user.id);

        event.from()->creator->message_get(event.command.message_id, event.command.channel_id, [this, event, &vote, user](const dpp::confirmation_callback_t& callback)
                                           {
                if (callback.is_error()) return;

                auto msg = callback.get<dpp::message>();
                if ((vote.m_voteAccept + vote.m_voteReject) >= 3) {
                    bool voteResult = (vote.m_voteAccept > vote.m_voteReject);
                    FinalizeVote(*event.from()->creator, msg, vote, voteResult);
                    m_activeVotes.erase(msg.id);
                } else {
                    msg.set_content(fmt::format("**Голосование:**\n✅ За: {}\n❌ Против: {}", 
                        vote.m_voteAccept, vote.m_voteReject));
                }
                
                event.from()->creator->message_edit(msg); });
    }
    event.reply();
    SaveState();
}

void CApplicationVoteSystem::ProcessFormSubmit(const dpp::form_submit_t& event)
{
    try
    {
        dpp::user user = event.command.get_issuing_user();
        std::string nickname = std::get<std::string>(event.components[0].components[0].value);
        std::string age = std::get<std::string>(event.components[1].components[0].value);
        std::string about = std::get<std::string>(event.components[2].components[0].value);

        if (!std::all_of(age.begin(), age.end(), ::isdigit))
        {
            event.reply(dpp::message("Возраст должен быть числом").set_flags(dpp::m_ephemeral));
            return;
        }

        int i_age = std::stoi(age);
        if (i_age < 1)
        {
            event.reply(dpp::message("Некорректный возраст").set_flags(dpp::m_ephemeral));
            return;
        }

        event.reply(dpp::message("Заявка отправлена").set_flags(dpp::m_ephemeral));
        std::string points = std::to_string(Parsing::GetPoints(Parsing::GetUrl(nickname)));

        CreateVoteMessage(*event.from()->creator, user, nickname, age, about, points);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка в ProcessFormSubmit: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке заявки");
    }
}

void CApplicationVoteSystem::CreateVoteMessage(dpp::cluster& bot, const dpp::user& user, const std::string& nickname, const std::string& age, const std::string& about, const std::string& points)
{
    dpp::embed embed = dpp::embed()
                           .set_author(user.username, "", user.get_avatar_url())
                           .set_color(dpp::colors::sti_blue)
                           .set_title("Новая заявка")
                           .add_field("Ник: ", nickname)
                           .add_field("Возраст: ", age)
                           .add_field("Поинты: ", points)
                           .add_field("О себе", about);

    dpp::message modsMsg(CHANNEL_MODERATION_ID, embed);

    dpp::component actionRow;
    actionRow.add_component(
        dpp::component()
            .set_label("Принять")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_success)
            .set_id("accept"));
    actionRow.add_component(
        dpp::component()
            .set_label("Отклонить")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("reject"));
    modsMsg.add_component(actionRow);

    bot.message_create(modsMsg, [this, user, nickname, age, about](const dpp::confirmation_callback_t& callback)
                       {
        if (callback.is_error()) return;
        
        auto msg = callback.get<dpp::message>();
        SApplicationVoteData voteData;
        voteData.m_targetUserId = user.id;
        voteData.m_userData = {
            {"game_nick", nickname},
            {"age", age},
            {"about", about},
            {"social_rating", 1000}
        };
        
        m_activeVotes[msg.id] = voteData;
        SaveState(); });
}

void CApplicationVoteSystem::FinalizeVote(dpp::cluster& bot, const dpp::message& msg, SApplicationVoteData& vote, bool voteResult)
{
    dpp::embed tmpEmbed = msg.embeds[0];
    tmpEmbed.set_color(voteResult ? dpp::colors::green : dpp::colors::red);

    dpp::message newMsg = msg;
    newMsg.set_content("");
    newMsg.embeds.clear();
    newMsg.add_embed(tmpEmbed);
    newMsg.components.clear();

    if (voteResult)
    {
        DataBase db(PATH_MEMBERS_DATA_BASE);
        db.SetUser(vote.m_targetUserId, vote.m_userData);
        db.Save();

        try
        {
            dpp::message directMsg;
            directMsg.set_content("Ваша заявка была одобрена!");
            bot.direct_message_create(vote.m_targetUserId, directMsg);
        }
        catch (const std::exception& e)
        {
            std::cout << "Ошибка отправки DM: " << e.what() << std::endl;
        }

        bot.guild_member_remove_role(msg.guild_id, vote.m_targetUserId, DEFAULT_ROLE_ID);
        bot.guild_member_add_role(msg.guild_id, vote.m_targetUserId, CLAN_ROLE_ID);
    }
}

void CApplicationVoteSystem::SaveState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data;

    for (auto& [msgId, vote] : m_activeVotes)
    {
        data[std::to_string(msgId)] = vote.ToJson();
    }

    db.SaveVoteData(data);
}

void CApplicationVoteSystem::LoadState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data = db.GetVoteData();

    if (!data.is_null())
    {
        m_activeVotes.clear();
        for (auto& [key, value] : data.items())
        {
            dpp::snowflake msgId = std::stoull(key);
            m_activeVotes[msgId] = SApplicationVoteData::FromJson(value);
        }
    }
}

nlohmann::json SApplicationVoteData::ToJson() const
{
    return {
        {"voteAccept", m_voteAccept},
        {"voteReject", m_voteReject},
        {"votedUsers", m_votedUsers},
        {"targetUserId", m_targetUserId},
        {"userData", m_userData}};
}

SApplicationVoteData SApplicationVoteData::FromJson(const nlohmann::json& j)
{
    SApplicationVoteData v;
    v.m_voteAccept = j.value("voteAccept", 0);
    v.m_voteReject = j.value("voteReject", 0);
    v.m_targetUserId = j.value("targetUserId", "");

    if (j.contains("userData"))
    {
        v.m_userData = j["userData"];
    }

    if (j.contains("votedUsers"))
    {
        for (const auto& id : j["votedUsers"])
        {
            v.m_votedUsers.insert(id.get<dpp::snowflake>());
        }
    }

    return v;
}
