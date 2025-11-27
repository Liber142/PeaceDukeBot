#include <dpp/application.h>
#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <fmt/format.h>
#include <algorithm>
#include <chrono>
#include <dpp/nlohmann/json_fwd.hpp>
#include <exception>
#include <iostream>
#include <string>
#include "../ConstAgr.h"
#include "../DataBase.h"
#include "../Parsing.h"
#include "ApplicationVote.h"

void CApplicationVoteSystem::Initialize(dpp::cluster& bot)
{
    LoadState();

    bot.on_button_click([this, &bot](const dpp::button_click_t& event)
                        {
        if(event.custom_id == "accept" || event.custom_id == "reject")
        {
            ProcessButtonClick(event);
        } });

    bot.on_form_submit([this](const dpp::form_submit_t& event)
                       { ProcessFormSubmit(event); });
}

void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    auto it = m_activeApplications.find(event.command.message_id);
    if (it == m_activeApplications.end())
    {
        event.reply("Заявка не найдена");
        return;
    }

    SApplicationVoteData& application = it->second;

    if (application.m_status != "pending")
    {
        event.reply("Заявка уже обработана");
        return;
    }

    // Проверяем, не голосовал ли уже этот модератор
    if (application.m_votes.find(event.command.usr.id) != application.m_votes.end())
    {
        event.reply("Вы уже проголосовали по этой заявке");
        return;
    }

    bool vote = (event.custom_id == "accept");
    ProcessVote(*event.from()->creator, event, application, vote);
}

void CApplicationVoteSystem::ProcessVote(dpp::cluster& bot, const dpp::button_click_t& event, 
                                        SApplicationVoteData& application, bool vote)
{
    // Записываем голос
    application.m_votes[event.command.usr.id] = vote;
    
    event.reply(dpp::message("Ваш голос учтен: " + std::string(vote ? "✅ Принято" : "❌ Отклонено"))
                .set_flags(dpp::m_ephemeral));
    
    // Проверяем результат голосования
    CheckVoteResult(bot, application);
}

void CApplicationVoteSystem::CheckVoteResult(dpp::cluster& bot, SApplicationVoteData& application)
{
    int acceptCount = 0;
    int rejectCount = 0;
    
    for (const auto& vote : application.m_votes)
    {
        if (vote.second) acceptCount++;
        else rejectCount++;
    }
    
    // Для принятия нужно 2 голоса "за", для отклонения достаточно 1 голоса "против"
    if (acceptCount >= 2)
    {
        AcceptApplication(bot, application);
    }
    else if (rejectCount >= 1)
    {
        RejectApplication(bot, application);
    }
    else
    {
        // Обновляем сообщение с текущими результатами
        UpdateApplicationMessage(bot, application);
    }
}

void CApplicationVoteSystem::AcceptApplication(dpp::cluster& bot, SApplicationVoteData& application)
{
    application.m_status = "accepted";
    
    // Выдаем роль
    AssignMemberRole(bot, application.m_targetUserId);
    
    // Отправляем сообщение пользователю
    bot.direct_message_create(
        application.m_targetUserId,
        dpp::message(defaultAcceptedDirectMessage));
    
    // Отправляем приветственное сообщение
    SendWelcomeMessage(bot, application);
    
    // Обновляем сообщение заявки
    UpdateApplicationMessage(bot, application);
    
    SaveState();
}

void CApplicationVoteSystem::RejectApplication(dpp::cluster& bot, SApplicationVoteData& application)
{
    application.m_status = "rejected";
    
    // Отправляем сообщение пользователю
    bot.direct_message_create(
        application.m_targetUserId,
        dpp::message(defaultRejectedDirectMessage));
    
    // Обновляем сообщение заявки
    UpdateApplicationMessage(bot, application);
    
    SaveState();
}

void CApplicationVoteSystem::UpdateApplicationMessage(dpp::cluster& bot, SApplicationVoteData& application)
{
    dpp::embed embed;
    
    if (application.m_status == "pending")
    {
        int acceptCount = 0;
        int rejectCount = 0;
        
        for (const auto& vote : application.m_votes)
        {
            if (vote.second) acceptCount++;
            else rejectCount++;
        }
        
        embed = dpp::embed()
            .set_color(dpp::colors::blue)
            .set_title("📝 Заявка в клан (голосование)")
            .add_field("Пользователь:", "<@" + std::to_string(application.m_targetUserId) + ">", true)
            .add_field("Никнейм:", application.m_NickName, true)
            .add_field("Возраст:", std::to_string(application.m_Age), true)
            .add_field("Социальный рейтинг:", std::to_string(application.m_SocialReting), true)
            .add_field("О себе:", application.m_About)
            .add_field("Голоса:", fmt::format("✅ {} | ❌ {}", acceptCount, rejectCount), true)
            .set_footer(dpp::embed_footer().set_text("Нужно 2 ✅ для принятия или 1 ❌ для отклонения"));
            
        // Добавляем кнопки только если заявка еще не обработана
        dpp::message msg(CHANNEL_MODERATION_ID, embed);
        msg.id = application.m_messageId;
        
        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label("✅ Принять")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_success)
                .set_id("accept"));
        actionRow.add_component(
            dpp::component()
                .set_label("❌ Отклонить")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_danger)
                .set_id("reject"));
                
        msg.components.clear();
        msg.add_component(actionRow);
        
        bot.message_edit(msg);
    }
    else
    {
        // Финальное сообщение с результатом
        embed = dpp::embed()
            .set_color(application.m_status == "accepted" ? dpp::colors::green : dpp::colors::red)
            .set_title(application.m_status == "accepted" ? "✅ Заявка принята" : "❌ Заявка отклонена")
            .add_field("Пользователь:", "<@" + std::to_string(application.m_targetUserId) + ">", true)
            .add_field("Никнейм:", application.m_NickName, true)
            .add_field("Возраст:", std::to_string(application.m_Age), true)
            .add_field("Результат:", application.m_status == "accepted" ? "Принята в клан" : "Отклонена", true);
            
        dpp::message msg(CHANNEL_MODERATION_ID, embed);
        msg.id = application.m_messageId;
        msg.components.clear(); // Убираем кнопки
        
        bot.message_edit(msg);
    }
}

void CApplicationVoteSystem::ProcessFormSubmit(const dpp::form_submit_t& event)
{
    try
    {
        dpp::user user = event.command.get_issuing_user();
        std::string nickname = std::get<std::string>(event.components[0].components[0].value);
        std::string age = std::get<std::string>(event.components[1].components[0].value);
        std::string about = std::get<std::string>(event.components[2].components[0].value);

        // Валидация возраста
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
        CreateApplicationMessage(*event.from()->creator, user, nickname, age, about, points);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка в ProcessFormSubmit: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке заявки");
    }
}

void CApplicationVoteSystem::CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, 
                                                    const std::string& nickname, const std::string& age, 
                                                    const std::string& about, const std::string& points)
{
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::blue)
        .set_title("📝 Новая заявка в клан")
        .add_field("Пользователь:", user.get_mention(), true)
        .add_field("Никнейм:", nickname, true)
        .add_field("Возраст:", age, true)
        .add_field("Социальный рейтинг:", points, true)
        .add_field("О себе:", about)
        .set_footer(dpp::embed_footer().set_text("ID: " + std::to_string(user.id)))
        .set_timestamp(std::time(0));

    dpp::component actionRow;
    actionRow.add_component(
        dpp::component()
            .set_label("✅ Принять")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_success)
            .set_id("accept"));
    actionRow.add_component(
        dpp::component()
            .set_label("❌ Отклонить")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("reject"));

    dpp::message msg(CHANNEL_MODERATION_ID, embed);
    msg.add_component(actionRow);

    bot.message_create(msg, [this, user, nickname, age, about, points](const dpp::confirmation_callback_t& callback)
                       {
        if (callback.is_error())
        {
            std::cerr << "Ошибка отправки заявки: " << callback.get_error().message << std::endl;
            return;
        }

        auto message = callback.get<dpp::message>();

        SApplicationVoteData application;
        application.m_targetUserId = user.id;
        application.m_messageId = message.id;
        application.m_NickName = nickname;
        application.m_Age = std::stoi(age);
        application.m_About = about;
        application.m_SocialReting = std::stoi(points);
        application.m_status = "pending";

        m_activeApplications[message.id] = application;
        SaveState(); });
}

void CApplicationVoteSystem::AssignMemberRole(dpp::cluster& bot, dpp::snowflake userId)
{
    bot.guild_member_add_role(GUILD_ID, userId, MEMBER_ROLE_ID, [userId](const dpp::confirmation_callback_t& callback)
                              {
        if (callback.is_error())
        {
            std::cerr << "Ошибка выдачи роли пользователю " << userId << ": "
                      << callback.get_error().message << std::endl;
        }
        else
        {
            std::cout << "Роль выдана пользователю " << userId << std::endl;
        } });
}

void CApplicationVoteSystem::SendWelcomeMessage(dpp::cluster& bot, const SApplicationVoteData& application)
{
    std::string welcomeMsg = fmt::format(
        "🎉 **Добро пожаловать в клан, {}!**\n\n{}",
        application.m_NickName,
        defaultWelcomeMessage);

    dpp::embed welcomeEmbed = dpp::embed()
        .set_title("Новый участник! 🎉")
        .set_description(welcomeMsg)
        .add_field("Никнейм:", application.m_NickName, true)
        .add_field("Возраст:", std::to_string(application.m_Age), true)
        .set_color(dpp::colors::green)
        .set_footer(dpp::embed_footer().set_text("Добро пожаловать!"));

    bot.message_create(dpp::message(WELCOME_CHANNEL_ID, welcomeEmbed));
}

void CApplicationVoteSystem::SaveState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data;

    for (auto& [msgId, application] : m_activeApplications)
    {
        data[std::to_string(msgId)] = application.ToJson();
    }

    db.SaveVoteData(data);
}

void CApplicationVoteSystem::LoadState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data = db.GetVoteData();

    // Загрузка конфигурации
    std::ifstream config(PATH_CONFIG);
    if (config.is_open())
    {
        try
        {
            nlohmann::json jsonConfig = nlohmann::json::parse(config);
            defaultAcceptedDirectMessage = jsonConfig.value("AplicationAceptedMessage", "🎉 Ваша заявка принята! Добро пожаловать в клан!");
            defaultRejectedDirectMessage = jsonConfig.value("AplicationRejectedMessage", "❌ К сожалению, ваша заявка была отклонена.");
            defaultWelcomeMessage = jsonConfig.value("WelcomeMessage", "Рады приветствовать нового участника в наших рядах!");
        }
        catch (const std::exception& e)
        {
            std::cerr << "Ошибка загрузки конфигурации: " << e.what() << std::endl;
        }
        config.close();
    }

    if (!data.is_null())
    {
        m_activeApplications.clear();
        for (auto& [key, value] : data.items())
        {
            try
            {
                dpp::snowflake msgId = std::stoull(key);
                SApplicationVoteData application = SApplicationVoteData::FromJson(value);
                application.m_messageId = msgId;
                m_activeApplications[msgId] = application;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Ошибка загрузки заявки " << key << ": " << e.what() << std::endl;
            }
        }
    }
}

// Реализация методов SApplicationVoteData
nlohmann::json SApplicationVoteData::ToJson() const
{
    nlohmann::json votesJson = nlohmann::json::object();
    for (const auto& vote : m_votes)
    {
        votesJson[std::to_string(vote.first)] = vote.second;
    }

    return {
        {"targetUserId", std::to_string(m_targetUserId)},
        {"messageId", std::to_string(m_messageId)},
        {"game_nick", m_NickName},
        {"age", m_Age},
        {"social_rating", m_SocialReting},
        {"about", m_About},
        {"status", m_status},
        {"votes", votesJson}
    };
}

SApplicationVoteData SApplicationVoteData::FromJson(const nlohmann::json& j)
{
    SApplicationVoteData v;

    if (j.contains("targetUserId"))
        v.m_targetUserId = std::stoull(j["targetUserId"].get<std::string>());
    if (j.contains("messageId"))
        v.m_messageId = std::stoull(j["messageId"].get<std::string>());
    if (j.contains("game_nick"))
        v.m_NickName = j["game_nick"];
    if (j.contains("age"))
        v.m_Age = j["age"];
    if (j.contains("social_rating"))
        v.m_SocialReting = j["social_rating"];
    if (j.contains("about"))
        v.m_About = j["about"];
    if (j.contains("status"))
        v.m_status = j["status"];

    if (j.contains("votes"))
    {
        for (auto& [key, value] : j["votes"].items())
        {
            dpp::snowflake userId = std::stoull(key);
            v.m_votes[userId] = value.get<bool>();
        }
    }

    return v;
}
