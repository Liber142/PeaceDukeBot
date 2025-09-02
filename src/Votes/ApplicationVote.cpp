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
        if (event.custom_id == "accept" || event.custom_id == "reject") {
            ProcessButtonClick(event);
        } else if (event.custom_id == "confirm_accept" || event.custom_id == "confirm_reject" ||
                   event.custom_id == "edit_dm" || event.custom_id == "custom_reason") {
            
            std::cout << m_activeMessangePair[event.command.message_id] << " | " << event.command.message_id << std::endl;
            auto it = m_activeApplications.find(m_activeMessangePair[event.command.message_id]);
            if (it == m_activeApplications.end()) {
                std::cout << "1" << std::endl;
                event.reply(dpp::message("Заявка не найдена").set_flags(dpp::m_ephemeral));
                return;
            }
            
            SApplicationVoteData& application = it->second;
            
            if (event.custom_id == "confirm_accept") {
                ProcessAcceptance(bot, event, application);
            } else if (event.custom_id == "confirm_reject") {
                ShowRejectionReasons(bot, event, application);
            } else if (event.custom_id == "edit_dm") {
                dpp::interaction_modal_response modal("edit_dm_msg", "Редактирование сообщения");
                modal.add_component(
                    dpp::component()
                        .set_label("Сообщение для пользователя")
                        .set_id("dm_message")
                        .set_type(dpp::cot_text)
                        .set_placeholder("Введите сообщение...")
                        .set_text_style(dpp::text_paragraph));
                
                event.dialog(modal);
            } else if (event.custom_id == "custom_reason") {
                dpp::interaction_modal_response modal("custom_reason", "Причина отказа");
                modal.add_component(
                    dpp::component()
                        .set_label("Причина отказа")
                        .set_id("rejection_reason")
                        .set_type(dpp::cot_text)
                        .set_placeholder("Укажите причину отказа...")
                        .set_text_style(dpp::text_paragraph));
                
                event.dialog(modal);
            }
        } else if (event.custom_id.find("reason_") == 0) {
            auto it = m_activeApplications.find(event.command.message_id);
            if (it == m_activeApplications.end()) {

                std::cout << "2" << std::endl;
                event.reply(dpp::message("Заявка не найдена").set_flags(dpp::m_ephemeral));
                return;
            }
            
            SApplicationVoteData& application = it->second;
            std::string reason = event.custom_id.substr(7);
            ProcessRejection(bot, event, application, reason);
        } });

    bot.on_form_submit([this, &bot](const dpp::form_submit_t& event)
                       {
        if (event.custom_id == "custom_reason") {
            std::string reason = std::get<std::string>(event.components[0].components[0].value);
            
            auto it = m_activeApplications.find(event.command.message_id);
            if (it != m_activeApplications.end()) {
                ProcessRejection(bot, event, it->second, reason);
            }
        } else if (event.custom_id == "edit_dm_msg") {
            std::string customMessage = std::get<std::string>(event.components[0].components[0].value);
            
            auto it = m_activeApplications.find(event.command.message_id);
            if (it != m_activeApplications.end()) {
                event.reply(dpp::message("Сообщение сохранено").set_flags(dpp::m_ephemeral));
            }
        } });
}

void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    auto it = m_activeApplications.find(event.command.message_id);
    if (it == m_activeApplications.end())
    {
        event.reply(dpp::message("Заявка не найдена или уже обработана").set_flags(dpp::m_ephemeral));
        return;
    }

    SApplicationVoteData& application = it->second;

    if (application.m_status != "pending")
    {
        event.reply(dpp::message("Эта заявка уже обработана").set_flags(dpp::m_ephemeral));
        return;
    }

    ShowModeratorOptions(*event.from()->creator, event, application);
}

void CApplicationVoteSystem::ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application)
{
    dpp::message msg("");
    msg.set_flags(dpp::m_ephemeral);

    if (event.custom_id == "accept")
    {
        msg.set_content("Выберите действие для принятия заявки:");

        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label("Подтвердить принятие")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_success)
                .set_id("confirm_accept"));
        actionRow.add_component(
            dpp::component()
                .set_label("Изменить сообщение")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("edit_dm"));

        msg.add_component(actionRow);
    }
    else if (event.custom_id == "reject")
    {
        msg.set_content("Выберите действие для отклонения заявки:");

        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label("Выбрать причину")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("show_reasons"));
        actionRow.add_component(
            dpp::component()
                .set_label("Своя причина")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_secondary)
                .set_id("custom_reason"));

        msg.add_component(actionRow);
    }

    m_activeMessangePair[msg.id] = event.command.message_id;
    std::cout << m_activeMessangePair[msg.id] << " | " << msg.id << std::endl;
    event.reply(msg);
}

void CApplicationVoteSystem::ShowRejectionReasons(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application)
{
    dpp::message msg("");
    msg.set_flags(dpp::m_ephemeral);
    msg.set_content("Выберите причину отказа:");

    dpp::component actionRow1;
    actionRow1.add_component(
        dpp::component()
            .set_label("Недостаточно опыта")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("reason_Недостаточно опыта"));
    actionRow1.add_component(
        dpp::component()
            .set_label("Не подходит по возрасту")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("reason_Не подходит по возрасту"));

    dpp::component actionRow2;
    actionRow2.add_component(
        dpp::component()
            .set_label("Неполная информация")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_danger)
            .set_id("reason_Неполная информация"));
    actionRow2.add_component(
        dpp::component()
            .set_label("Другая причина")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_secondary)
            .set_id("custom_reason"));

    msg.add_component(actionRow1);
    msg.add_component(actionRow2);

    event.reply(msg);
}

void CApplicationVoteSystem::ProcessAcceptance(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application)
{
    application.m_status = "accepted";
    application.m_processedBy = event.command.get_issuing_user().id;

    SApplicationVoteData appCopy = application;

    bot.message_get(event.command.message_id, event.command.channel_id, [this, &bot, appCopy](const dpp::confirmation_callback_t& callback) mutable
                    {
            if (callback.is_error()) return;
            
            auto msg = callback.get<dpp::message>();
            FinalizeApplication(bot, msg, appCopy, true); });

    event.reply(dpp::message("Заявка принята").set_flags(dpp::m_ephemeral));
    SaveState();
}

void CApplicationVoteSystem::ProcessRejection(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application, const std::string& reason)
{
    DoRejection(bot, event.command.message_id, event.command.channel_id, application, reason, event.command.get_issuing_user().id);
    event.reply(dpp::message("Заявка отклонена").set_flags(dpp::m_ephemeral));
}

void CApplicationVoteSystem::ProcessRejection(dpp::cluster& bot, const dpp::form_submit_t& event, SApplicationVoteData& application, const std::string& reason)
{
    DoRejection(bot, event.command.message_id, event.command.channel_id, application, reason, event.command.get_issuing_user().id);
    event.reply(dpp::message("Заявка отклонена").set_flags(dpp::m_ephemeral));
}

void CApplicationVoteSystem::DoRejection(dpp::cluster& bot, dpp::snowflake message_id, dpp::snowflake channel_id, SApplicationVoteData& application, const std::string& reason, dpp::snowflake moderator_id)
{
    application.m_status = "rejected";
    application.m_processedBy = moderator_id;
    application.m_rejectionReason = reason;

    SApplicationVoteData appCopy = application;

    bot.message_get(message_id, channel_id, [this, &bot, appCopy, reason](const dpp::confirmation_callback_t& callback) mutable
                    {
            if (callback.is_error()) return;
            
            auto msg = callback.get<dpp::message>();
            FinalizeApplication(bot, msg, appCopy, false, reason); });

    SaveState();
}

void CApplicationVoteSystem::FinalizeApplication(dpp::cluster& bot, const dpp::message& msg, SApplicationVoteData& application, bool accepted, const std::string& reason)
{
    dpp::embed tmpEmbed = msg.embeds[0];
    tmpEmbed.set_color(accepted ? dpp::colors::green : dpp::colors::red);

    std::string moderatorInfo = fmt::format("Обработано: <@{}>", application.m_processedBy);
    if (!accepted && !reason.empty())
    {
        moderatorInfo += fmt::format("\nПричина: {}", reason);
    }

    tmpEmbed.add_field("Статус", accepted ? "✅ Одобрена" : "❌ Отклонена", true);
    tmpEmbed.add_field("Модератор", moderatorInfo, false);

    dpp::message newMsg = msg;
    newMsg.set_content("");
    newMsg.embeds.clear();
    newMsg.add_embed(tmpEmbed);
    newMsg.components.clear();

    bot.message_edit(newMsg);

    if (accepted)
    {
        DataBase db(PATH_MEMBERS_DATA_BASE);
        db.SetUser(application.m_targetUserId, application.m_userData);
        db.Save();

        try
        {
            dpp::message directMsg;
            directMsg.set_content("Ваша заявка в клан была одобрена! Добро пожаловать!");
            bot.direct_message_create(application.m_targetUserId, directMsg);
        }
        catch (const std::exception& e)
        {
            std::cout << "Ошибка отправки DM: " << e.what() << std::endl;
        }

        bot.guild_member_remove_role(msg.guild_id, application.m_targetUserId, DEFAULT_ROLE_ID);
        bot.guild_member_add_role(msg.guild_id, application.m_targetUserId, CLAN_ROLE_ID);
    }
    else
    {
        try
        {
            dpp::message directMsg;
            std::string rejectionMessage = fmt::format("Ваша заявка в клан была отклонена.\n{}",
                                                       reason.empty() ? "" : "Причина: " + reason);
            directMsg.set_content(rejectionMessage);
            bot.direct_message_create(application.m_targetUserId, directMsg);
        }
        catch (const std::exception& e)
        {
            std::cout << "Ошибка отправки DM: " << e.what() << std::endl;
        }
    }

    m_activeApplications.erase(msg.id);
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

        CreateApplicationMessage(*event.from()->creator, user, nickname, age, about, points);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка в ProcessFormSubmit: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке заявки");
    }
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
        voteData.m_status = "pending";
        
        voteData.m_userData = {
            {"game_nick", nickname},
            {"age", age},
            {"about", about},
            {"social_rating", 1000}
        };
        
        m_activeApplications[msg.id] = voteData;
        SaveState(); });
}

void CApplicationVoteSystem::SaveState()
{
    DataBase db(PATH_VOTES_DATA_BASE);
    nlohmann::json data;

    for (auto& [msgId, vote] : m_activeApplications)
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
        m_activeApplications.clear();
        for (auto& [key, value] : data.items())
        {
            dpp::snowflake msgId = std::stoull(key);
            m_activeApplications[msgId] = SApplicationVoteData::FromJson(value);
        }
    }
}

nlohmann::json SApplicationVoteData::ToJson() const
{
    return {
        {"targetUserId", m_targetUserId},
        {"processedBy", m_processedBy},
        {"userData", m_userData},
        {"status", m_status},
        {"rejectionReason", m_rejectionReason}};
}

SApplicationVoteData SApplicationVoteData::FromJson(const nlohmann::json& j)
{
    SApplicationVoteData v;
    v.m_targetUserId = j.value("targetUserId", "");
    v.m_processedBy = j.value("processedBy", "");
    v.m_status = j.value("status", "pending");
    v.m_rejectionReason = j.value("rejectionReason", "");

    if (j.contains("userData"))
    {
        v.m_userData = j["userData"];
    }

    return v;
}
