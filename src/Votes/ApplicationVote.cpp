#include <dpp/application.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <fmt/format.h>
#include <algorithm>
#include <dpp/nlohmann/json_fwd.hpp>
#include <exception>
#include <iostream>
#include <ostream>
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
            if(event.custom_id == "accept" || 
                    event.custom_id == "reject" ||
                    event.custom_id.find("edit") != std::string::npos ||
                    event.custom_id.find("confirm") != std::string::npos)    
            {
                ProcessButtonClick(event);
            } });
}

void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    dpp::snowflake id;
    if (event.custom_id.find(":") != std::string::npos)
    {
        id = event.custom_id.substr(event.custom_id.find(":") + 1);
        std::cout << id << " | " << event.custom_id << std::endl;
    }
    else
    {
        id = event.command.message_id;
    }
    auto it = m_activeApplications.find(id);
    if (it == m_activeApplications.end())
    {
        event.reply("Заявка не найдена");
        return;
    }

    SApplicationVoteData& application = it->second;
    application.m_direckMessage = event.custom_id == "accept" ? defaultAcceptedDirectMessage : defaultRejectedDirectMessage;

    if (event.custom_id.find("edit") != std::string::npos)
    {
        std::cout << "edit direct message" << std::endl;
        ShowEditModal(event, application);
    }
    if (application.m_status == "pending" && (event.custom_id == "accept" || event.custom_id == "reject"))
    {
        ShowModeratorOptions(*event.from()->creator, event, application);
    }
}

void CApplicationVoteSystem::ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application)
{
    std::string strMsg;
    strMsg += "Пользователю в личные сообщения отправиться следующее сообщение: \n";
    strMsg += "```\n" + application.m_direckMessage + "\n```\n";
    strMsg += "убедитесь в коректности параметров или измените их\n";
    strMsg += "Подтвердите своё решение\n";
    dpp::message msg = dpp::message(CHANNEL_MODERATION_ID, strMsg).set_flags(dpp::m_ephemeral);

    bool result = event.custom_id == "accept";
    std::string button_id = result ? "confirm_accept:" : "confirm_reject:";
    button_id += std::to_string(event.command.message_id);
    dpp::component actionRow;
    actionRow.add_component(dpp::component(
        dpp::component()
            .set_label("Подтвердить")
            .set_type(dpp::cot_button)
            .set_style(result ? dpp::cos_success : dpp::cos_danger)
            .set_id(button_id)));

    button_id = "edit:" + std::to_string(event.command.message_id);
    actionRow.add_component(dpp::component(
        dpp::component()
            .set_label("Редактировать")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_primary)
            .set_id(button_id)));

    if (!result)
    {
        actionRow.add_component(dpp::component(
            dpp::component()
                .set_label("Причина")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_danger)
                .set_id("reason")));

        actionRow.add_component(dpp::component(
            dpp::component()
                .set_label("В черный список")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_danger)
                .set_id("blacklist")));
    }

    msg.add_component(actionRow);
    event.reply(msg);
}

void CApplicationVoteSystem::ShowEditModal(const dpp::button_click_t& event, SApplicationVoteData application)
{
    dpp::interaction_modal_response modal("editor_modal", "Изменение ответа на заявку");

    modal.add_component(
            dpp::component()
                .set_label("Сообщение в ЛС")
                .set_id("dm")
                .set_type(dpp::cot_text)
                .set_default_value(application.m_direckMessage)
                .set_text_style(dpp::text_paragraph)
            );
    modal.add_row();
    event.dialog(modal);
}

void CApplicationVoteSystem::CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, const std::string& nickname, const std::string& age, const std::string& about, const std::string& points)
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

    // Добавление кнопок принятия/отклонения
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
        if (callback.is_error())
            return;

        auto msg = callback.get<dpp::message>();
        SApplicationVoteData applicationData;
        applicationData.m_targetUserId = user.id;
        applicationData.m_NickName = nickname;
        applicationData.m_Age = std::stoi(age);
        applicationData.m_About = about;
        applicationData.m_status = "pending";

        m_activeApplications[msg.id] = applicationData;
        SaveState(); });
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

// Загрузка состояния заявок
void CApplicationVoteSystem::LoadState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data = db.GetVoteData();
    std::ifstream config(PATH_CONFIG);
    nlohmann::json jsonConfig = nlohmann::json::parse(config);
    if (jsonConfig.contains("AplicationAceptedMessage"))
        defaultAcceptedDirectMessage = jsonConfig.value("AplicationAceptedMessage", "");
    if (jsonConfig.contains("AplicationRejectedMessage"))
        defaultRejectedDirectMessage = jsonConfig.value("AplicationRejectedMessage", "");
    config.close();

    if (!data.is_null())
    {
        m_activeApplications.clear();
        for (auto& [key, value] : data.items())
        {
            dpp::snowflake msgId = std::stoull(key);
            m_activeApplications[msgId] = SApplicationVoteData::FromJson(value);
        }
    }
}

// Сериализация данных заявки
nlohmann::json SApplicationVoteData::ToJson() const
{
    return {
        {"targetUserId", m_targetUserId},
        {"processedBy", m_processedBy},
        {"game_nick", m_NickName},
        {"age", m_Age},
        {"social_rating", m_SocialReting},
        {"about", m_About},
        {"status", m_status},
        {"rejectionReason", m_rejectionReason}};
}

// Десериализация данных заявки
SApplicationVoteData SApplicationVoteData::FromJson(const nlohmann::json& j)
{
    SApplicationVoteData v;
    if (j.contains("targetUserId"))
        v.m_targetUserId = j.value("targetUserId", "");
    if (j.contains("processesBy"))
        v.m_processedBy = j.value("processedBy", "");
    if (j.contains("status"))
        v.m_status = j.value("status", "");
    if (j.contains("rejectionReason"))
        v.m_rejectionReason = j.value("rejectionReason", "");

    if (j.contains("userData"))
    {
        v.m_NickName = j["userData"].value("game_nick", "");
        v.m_Age = std::stoi(j["userData"].value("age", ""));
        v.m_About = j["userData"].value("about", "");
        v.m_SocialReting = j["userData"].value("social_rating", 0);
    }
    else
    {
        if (j.contains("game_nick"))
            v.m_NickName = j.value("game_nick", "");
        if (j.contains("age"))
            v.m_Age = j.value("age", 0);
        if (j.contains("about"))
            v.m_About = j.value("about", "");
        if (j.contains("social_rating"))
            v.m_SocialReting = j.value("social_rating", 0);
    }

    return v;
}
