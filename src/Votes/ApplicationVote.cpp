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
           event.custom_id.find("confirm") != std::string::npos ||
           event.custom_id.find("reason") != std::string::npos ||
           event.custom_id.find("blacklist") != std::string::npos ||
           event.custom_id.find("intervene") != std::string::npos)    
        {
            ProcessButtonClick(event);
        }
    });

    // Обработка модальных окон
    bot.on_form_submit([this](const dpp::form_submit_t& event)
    {
        if (event.custom_id.find("modal_") != std::string::npos)
        {
            ProcessModalResponse(event);
        }
        else
        {
            ProcessFormSubmit(event);
        }
    });
}

void CApplicationVoteSystem::ProcessModalResponse(const dpp::form_submit_t& event)
{
    try
    {
        dpp::snowflake message_id;
        if (event.custom_id.find(":") != std::string::npos)
        {
            message_id = std::stoull(event.custom_id.substr(event.custom_id.find(":") + 1));
        }
        else
        {
            event.reply("Ошибка: не найден ID заявки");
            return;
        }

        auto it = m_activeApplications.find(message_id);
        if (it == m_activeApplications.end())
        {
            event.reply("Заявка не найдена");
            return;
        }

        SApplicationVoteData& application = it->second;

        if (event.custom_id.find("modal_edit") != std::string::npos)
        {
            // Редактирование сообщения в ЛС
            std::string new_dm = std::get<std::string>(event.components[0].components[0].value);
            application.m_direckMessage = new_dm;
            event.reply(dpp::message("Сообщение обновлено").set_flags(dpp::m_ephemeral));
        }
        else if (event.custom_id.find("modal_reason") != std::string::npos)
        {
            // Указание причины отказа
            std::string reason = std::get<std::string>(event.components[0].components[0].value);
            application.m_rejectionReason = reason;
            application.m_direckMessage = defaultRejectedDirectMessage + "\n\n**Причина:** " + reason;
            event.reply(dpp::message("Причина отказа добавлена").set_flags(dpp::m_ephemeral));
        }
        else if (event.custom_id.find("modal_blacklist") != std::string::npos)
        {
            // Добавление в черный список
            std::string reason = std::get<std::string>(event.components[0].components[0].value);
            application.m_rejectionReason = reason;
            application.m_isBlacklisted = true;
            application.m_direckMessage = "❌ Ваша заявка была отклонена и вы добавлены в черный список.\n**Причина:** " + reason;
            event.reply(dpp::message("Пользователь добавлен в черный список").set_flags(dpp::m_ephemeral));
        }

        SaveState();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка в ProcessModalResponse: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке данных");
    }
}

void CApplicationVoteSystem::ShowEditModal(const dpp::button_click_t& event, SApplicationVoteData application)
{
    dpp::interaction_modal_response modal("modal_edit:" + std::to_string(application.m_messageId), "Редактирование сообщения");

    modal.add_component(
        dpp::component()
            .set_label("Сообщение для пользователя")
            .set_id("dm_message")
            .set_type(dpp::cot_text)
            .set_default_value(application.m_direckMessage)
            .set_text_style(dpp::text_paragraph)
            .set_placeholder("Введите сообщение которое получит пользователь")
            .set_min_length(10)
            .set_max_length(2000)
    );

    event.dialog(modal);
}

void CApplicationVoteSystem::ShowReasonModal(const dpp::button_click_t& event, SApplicationVoteData application)
{
    dpp::interaction_modal_response modal("modal_reason:" + std::to_string(application.m_messageId), "Указание причины отказа");

    modal.add_component(
        dpp::component()
            .set_label("Причина отказа")
            .set_id("reason")
            .set_type(dpp::cot_text)
            .set_default_value(application.m_rejectionReason)
            .set_text_style(dpp::text_paragraph)
            .set_placeholder("Объясните причину отказа пользователю")
            .set_min_length(10)
            .set_max_length(1000)
    );

    event.dialog(modal);
}

void CApplicationVoteSystem::ShowBlacklistModal(const dpp::button_click_t& event, SApplicationVoteData application)
{
    dpp::interaction_modal_response modal("modal_blacklist:" + std::to_string(application.m_messageId), "Добавление в черный список");

    modal.add_component(
        dpp::component()
            .set_label("Причина добавления в ЧС")
            .set_id("blacklist_reason")
            .set_type(dpp::cot_text)
            .set_text_style(dpp::text_paragraph)
            .set_placeholder("Укажите причину добавления в черный список")
            .set_min_length(10)
            .set_max_length(1000)
            .set_required(true)
    );

    event.dialog(modal);
}

void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    dpp::snowflake id;
    if (event.custom_id.find(":") != std::string::npos)
    {
        id = event.custom_id.substr(event.custom_id.find(":") + 1);
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

    // Обработка разных типов кнопок
    if (event.custom_id.find("confirm_accept") != std::string::npos)
    {
        ProcessConfirmation(*event.from()->creator, event, application, true);
    }
    else if (event.custom_id.find("confirm_reject") != std::string::npos)
    {
        ProcessConfirmation(*event.from()->creator, event, application, false);
    }
    else if (event.custom_id.find("edit") != std::string::npos)
    {
        ShowEditModal(event, application);
    }
    else if (event.custom_id.find("reason") != std::string::npos && !application.m_isBlacklisted)
    {
        ShowReasonModal(event, application);
    }
    else if (event.custom_id.find("blacklist") != std::string::npos)
    {
        ShowBlacklistModal(event, application);
    }
    else if (event.custom_id.find("intervene") != std::string::npos &&
             application.m_status == "accepted" && !application.m_isBlacklisted)
    {
        CreateDiscussionChannel(*event.from()->creator, application);
    }
    else if (application.m_status == "pending" &&
             (event.custom_id == "accept" || event.custom_id == "reject"))
    {
        ShowModeratorOptions(*event.from()->creator, event, application);
    }
    else
    {
        event.reply(dpp::message("Дейстиве недоступно").set_flags(dpp::m_ephemeral));
    }
}


void CApplicationVoteSystem::ProcessConfirmation(dpp::cluster& bot, const dpp::button_click_t& event,
                                               SApplicationVoteData& application, bool accepted)
{
    application.m_status = accepted ? "accepted" : "rejected";
    application.m_processedBy = event.command.usr.id;
    application.m_decisionTime = std::chrono::system_clock::now();

    // Отправляем сообщение пользователю
    if (!application.m_direckMessage.empty())
    {
        bot.direct_message_create(application.m_targetUserId,
            dpp::message(application.m_direckMessage));
    }

    // Обновляем сообщение заявки
    dpp::message newMsg = event.command.msg;

    if (accepted)
    {
        newMsg.embeds[0].set_color(dpp::colors::yellow)
                        .set_title("✅ Заявка принята (ожидает подтверждения)")
                        .add_field("Принял:", event.command.usr.get_mention(), true)
                        .add_field("Статус:", "Автоподтверждение через 24 часа", true);

        // Добавляем кнопку "Вмешаться" только для принятых заявок
        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label("🚨 Вмешаться")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_secondary)
                .set_id("intervene:" + std::to_string(application.m_messageId))
        );
        newMsg.components.clear();
        newMsg.add_component(actionRow);

        // Устанавливаем таймер на 24 часа для финального принятия
        bot.start_timer([this, &bot, application](dpp::timer timer) {
            auto it = m_activeApplications.find(application.m_messageId);
            if (it != m_activeApplications.end() && it->second.m_status == "accepted")
            {
                ProcessFinalAcceptance(bot, it->second);
            }
        }, 86400); // 24 часа
    }
    else
    {
        // Для отклоненных заявок
        newMsg.embeds[0].set_color(dpp::colors::red)
                        .set_title("❌ Заявка отклонена")
                        .add_field("Отклонил:", event.command.usr.get_mention(), true);

        if (!application.m_rejectionReason.empty())
        {
            newMsg.embeds[0].add_field("Причина:", application.m_rejectionReason);
        }

        if (application.m_isBlacklisted)
        {
            newMsg.embeds[0].add_field("Черный список:", "✅ Да");
        }

        // Убираем все кнопки для отклоненных заявок
        newMsg.components.clear();

        // Архивируем отклоненную заявку сразу
        ArchiveApplication(bot, application);
    }

    bot.message_edit(newMsg);
    SaveState();
    event.reply(dpp::message("Решение применено").set_flags(dpp::m_ephemeral));
}

void CApplicationVoteSystem::ProcessFinalAcceptance(dpp::cluster& bot, SApplicationVoteData& application)
{
    // Выдаем роль и отправляем приветствие
    AssignMemberRole(bot, application.m_targetUserId);
    SendWelcomeMessage(bot, application);

    // Обновляем сообщение заявки
    dpp::message newMsg;
    newMsg.set_channel_id(CHANNEL_MODERATION_ID);
    newMsg.id = application.m_messageId;
    newMsg.embeds.push_back(
        dpp::embed()
            .set_color(dpp::colors::green)
            .set_title("✅ Заявка завершена")
            .add_field("Пользователь:", "<@" + std::to_string(application.m_targetUserId) + ">")
            .add_field("Ник:", application.m_NickName)
            .add_field("Принял:", "<@" + std::to_string(application.m_processedBy) + ">")
            .set_footer(dpp::embed_footer().set_text("Автоподтверждение"))
    );

    newMsg.components.clear(); // Убираем все кнопки

    bot.message_edit(newMsg);
    ArchiveApplication(bot, application);
}

void CApplicationVoteSystem::ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, SApplicationVoteData& application)
{
    std::string strMsg;
    strMsg += "Пользователю в личные сообщения отправится следующее сообщение: \n";
    strMsg += "```\n" + application.m_direckMessage + "\n```\n";

    if (!application.m_rejectionReason.empty())
    {
        strMsg += "**Причина отказа:** " + application.m_rejectionReason + "\n\n";
    }

    if (application.m_isBlacklisted)
    {
        strMsg += "⚠️ **Пользователь будет добавлен в черный список**\n\n";
    }

    strMsg += "Убедитесь в корректности параметров или измените их\n";
    strMsg += "Подтвердите своё решение\n";

    dpp::message msg = dpp::message(event.command.channel_id, strMsg).set_flags(dpp::m_ephemeral);

    bool is_accept = event.custom_id == "accept";
    std::string button_id = is_accept ? "confirm_accept:" : "confirm_reject:";
    button_id += std::to_string(application.m_messageId);

    dpp::component actionRow;
    actionRow.add_component(
        dpp::component()
            .set_label(is_accept ? "✅ Подтвердить принятие" : "❌ Подтвердить отклонение")
            .set_type(dpp::cot_button)
            .set_style(is_accept ? dpp::cos_success : dpp::cos_danger)
            .set_id(button_id));

    // Кнопка редактирования сообщения
    actionRow.add_component(
        dpp::component()
            .set_label("✏️ Редактировать")
            .set_type(dpp::cot_button)
            .set_style(dpp::cos_primary)
            .set_id("edit:" + std::to_string(application.m_messageId)));

    if (!is_accept)
    {
        // Для отклонения - дополнительные опции
        actionRow.add_component(
            dpp::component()
                .set_label("📝 Причина")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_secondary)
                .set_id("reason:" + std::to_string(application.m_messageId)));

        actionRow.add_component(
            dpp::component()
                .set_label("🚫 В черный список")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_danger)
                .set_id("blacklist:" + std::to_string(application.m_messageId)));
    }

    msg.add_component(actionRow);
    event.reply(msg);
}

nlohmann::json SApplicationVoteData::ToJson() const
{
    auto time_t = std::chrono::system_clock::to_time_t(m_decisionTime);

    return {
        {"targetUserId", m_targetUserId},
        {"processedBy", m_processedBy},
        {"messageId", m_messageId},
        {"discussionChannelId", m_discussionChannelId},
        {"game_nick", m_NickName},
        {"age", m_Age},
        {"social_rating", m_SocialReting},
        {"about", m_About},
        {"directMessage", m_direckMessage},
        {"status", m_status},
        {"rejectionReason", m_rejectionReason},
        {"decisionTime", time_t},
        {"isBlacklisted", m_isBlacklisted}
    };
}

SApplicationVoteData SApplicationVoteData::FromJson(const nlohmann::json& j)
{
    SApplicationVoteData v;

    // ... существующий код ...

    if (j.contains("messageId"))
        v.m_messageId = j.value("messageId", dpp::snowflake(0));
    if (j.contains("directMessage"))
        v.m_direckMessage = j.value("directMessage", "");
    if (j.contains("decisionTime"))
    {
        auto time_t = j.value("decisionTime", std::time_t{});
        v.m_decisionTime = std::chrono::system_clock::from_time_t(time_t);
    }
    if (j.contains("isBlacklisted"))
        v.m_isBlacklisted = j.value("isBlacklisted", false);

    return v;
}

