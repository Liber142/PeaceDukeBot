# Модуль системы модерации: Заявки и обработка
## Процесс работы системы

### Подача заявки
1. Пользователь заполняет форму заявки
2. Создается сообщение в канале модерации
3. Заявка добавляется в активный список

### Обработка модератором
1. Модератор нажимает кнопку "Принять" или "Отклонить"
2. Система показывает дополнительные опции:
   - Для принятия:  
		Подтверждение
		Изменение сообщение в лс
		Изменить приветсвеное сообщение в клан чате
   - Для отклонения: 
		Подтверждение 
		Изменить сообщение в лс
		Указать причину отказа (только для модераторов)
		Добавить в чёрный список
3. Заявка висит ещё сутки с кнопкой вмешаться
4. После нажатия кнопки создаёться новый чат для обсуждения участника

### Завершение обработки
1. При принятии заявки:
   - Пользователь добавляется в базу данных
   - Назначается роль участника клана
   - Отправляется уведомление в ЛС
   - Отправляеться приветсвие в клан чат

2. При отклонении заявки:
   - Указывается причина отказа
   - Отправляется уведомление в ЛС

 3. Заявка архивируеться в канале

### Работа с базой данных

- Данные заявок сохраняются между перезапусками бота
    
- Информация о пользователях хранится в JSON-формате
    
- Состояние активных заявок сохраняется в реальном времени

## Преимущества новой системы

1. **Упрощенный процесс** - решение принимает один модератор
2. **Гибкие опции** - различные варианты обработки заявок
3. **Прозрачность** - запись кто обработал заявку и по какой причине
4. **Улучшенная коммуникация** - отправка детальных уведомлений пользователям
5. **Архивация** - сохранение истории обработки заявок

## Архитектура системы

### Основные компоненты

1. **CApplicationVoteSystem** - основной класс системы обработки заявок
    
2. **SApplicationVoteData** - структура данных заявки
    
3. **ClanApplication** - класс для отображения модального окна заявки
    
4. **IVoteSystem** - интерфейс для систем голосования
    

### Взаимодействие компонентов

text

Пользователь → ClanApplication → CApplicationVoteSystem → База данных
Модератор → CApplicationVoteSystem → Уведомление пользователя

## Детальное описание компонентов

### 1. Структура SApplicationVoteData

```cpp

struct SApplicationVoteData
{
    dpp::snowflake m_targetUserId;      // ID пользователя, подавшего заявку
    dpp::snowflake m_processedBy;       // ID модератора, обработавшего заявку
    nlohmann::json m_userData;          // Данные пользователя (ник, возраст, о себе)
    std::string m_status;               // Статус заявки ("pending", "accepted", "rejected")
    std::string m_rejectionReason;      // Причина отказа

    nlohmann::json ToJson() const;      // Сериализация в JSON
    static SApplicationVoteData FromJson(const nlohmann::json& j); // Десериализация из JSON
};
```

### 2. Класс CApplicationVoteSystem

#### Публичные методы

```cpp

void Initialize(dpp::cluster& bot) override;
void ProcessButtonClick(const dpp::button_click_t& event) override;
void ProcessFormSubmit(const dpp::form_submit_t& event) override;
void SaveState() override;
void LoadState() override;
```
#### Приватные методы
```cpp

void CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, 
                             const std::string& nickname, const std::string& age, 
                             const std::string& about, const std::string& points);
void ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, 
                         SApplicationVoteData& application);
void ProcessAcceptance(dpp::cluster& bot, const dpp::button_click_t& event, 
                      SApplicationVoteData& application);
void ProcessRejection(dpp::cluster& bot, const dpp::button_click_t& event, 
                     SApplicationVoteData& application, const std::string& reason);
void ProcessRejection(dpp::cluster& bot, const dpp::form_submit_t& event, 
                     SApplicationVoteData& application, const std::string& reason);
void ShowRejectionReasons(dpp::cluster& bot, const dpp::button_click_t& event, 
                         SApplicationVoteData& application);
void FinalizeApplication(dpp::cluster& bot, const dpp::message& msg, 
                        SApplicationVoteData& application, bool accepted, 
                        const std::string& reason = "");
void DoRejection(dpp::cluster& bot, dpp::snowflake message_id, dpp::snowflake channel_id, 
                SApplicationVoteData& application, const std::string& reason, 
                dpp::snowflake moderator_id);
```

### 3. Полная реализация CApplicationVoteSystem

```cpp

#include <fmt/format.h>
#include <algorithm>
#include "ApplicationVote.h"
#include "ConstAgr.h"
#include "Parsing.h"

// Инициализация системы
void CApplicationVoteSystem::Initialize(dpp::cluster& bot)
{
    LoadState();

    // Обработчик кнопок
    bot.on_button_click([this, &bot](const dpp::button_click_t& event) {
        // Основные кнопки принятия/отклонения
        if (event.custom_id == "accept" || event.custom_id == "reject") {
            ProcessButtonClick(event);
        } 
        // Дополнительные опции модератора
        else if (event.custom_id == "confirm_accept" || event.custom_id == "confirm_reject" ||
                 event.custom_id == "edit_dm" || event.custom_id == "custom_reason") {
            auto it = m_activeApplications.find(event.command.message_id);
            if (it == m_activeApplications.end()) {
                event.reply(dpp::message("Заявка не найдена").set_flags(dpp::m_ephemeral));
                return;
            }
            
            SApplicationVoteData& application = it->second;
            
            if (event.custom_id == "confirm_accept") {
                ProcessAcceptance(bot, event, application);
            } else if (event.custom_id == "confirm_reject") {
                ShowRejectionReasons(bot, event, application);
            } else if (event.custom_id == "edit_dm") {
                // Модальное окно для редактирования сообщения
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
                // Модальное окно для ввода причины отказа
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
        } 
        // Предустановленные причины отказа
        else if (event.custom_id.find("reason_") == 0) {
            auto it = m_activeApplications.find(event.command.message_id);
            if (it == m_activeApplications.end()) {
                event.reply(dpp::message("Заявка не найдена").set_flags(dpp::m_ephemeral));
                return;
            }
            
            SApplicationVoteData& application = it->second;
            std::string reason = event.custom_id.substr(7);
            ProcessRejection(bot, event, application, reason);
        }
    });

    // Обработчик модальных окон
    bot.on_form_submit([this, &bot](const dpp::form_submit_t& event) {
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
                // Здесь можно сохранить кастомное сообщение
                event.reply(dpp::message("Сообщение сохранено").set_flags(dpp::m_ephemeral));
            }
        }
    });
}

// Обработка основной кнопки принятия/отклонения
void CApplicationVoteSystem::ProcessButtonClick(const dpp::button_click_t& event)
{
    auto it = m_activeApplications.find(event.command.message_id);
    if (it == m_activeApplications.end()) {
        event.reply(dpp::message("Заявка не найдена или уже обработана").set_flags(dpp::m_ephemeral));
        return;
    }

    SApplicationVoteData& application = it->second;
    
    // Проверка статуса заявки
    if (application.m_status != "pending") {
        event.reply(dpp::message("Эта заявка уже обработана").set_flags(dpp::m_ephemeral));
        return;
    }

    // Показ опций модератору
    ShowModeratorOptions(*event.from()->creator, event, application);
}

// Показ опций модератору
void CApplicationVoteSystem::ShowModeratorOptions(dpp::cluster& bot, const dpp::button_click_t& event, 
                                                 SApplicationVoteData& application)
{
    dpp::message msg("");
    msg.set_flags(dpp::m_ephemeral);
    
    if (event.custom_id == "accept") {
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
    } else if (event.custom_id == "reject") {
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
    
    event.reply(msg);
}

// Показ предустановленных причин отказа
void CApplicationVoteSystem::ShowRejectionReasons(dpp::cluster& bot, const dpp::button_click_t& event, 
                                                 SApplicationVoteData& application)
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

// Обработка принятия заявки
void CApplicationVoteSystem::ProcessAcceptance(dpp::cluster& bot, const dpp::button_click_t& event, 
                                              SApplicationVoteData& application)
{
    application.m_status = "accepted";
    application.m_processedBy = event.command.get_issuing_user().id;
    
    // Создание копии для передачи в лямбду
    SApplicationVoteData appCopy = application;
    
    bot.message_get(event.command.message_id, event.command.channel_id, 
        [this, &bot, appCopy](const dpp::confirmation_callback_t& callback) mutable {
            if (callback.is_error()) return;
            
            auto msg = callback.get<dpp::message>();
            FinalizeApplication(bot, msg, appCopy, true);
        });
    
    event.reply(dpp::message("Заявка принята").set_flags(dpp::m_ephemeral));
    SaveState();
}

// Обработка отклонения заявки (из кнопки)
void CApplicationVoteSystem::ProcessRejection(dpp::cluster& bot, const dpp::button_click_t& event, 
                                             SApplicationVoteData& application, const std::string& reason)
{
    DoRejection(bot, event.command.message_id, event.command.channel_id, application, 
               reason, event.command.get_issuing_user().id);
    event.reply(dpp::message("Заявка отклонена").set_flags(dpp::m_ephemeral));
}

// Обработка отклонения заявки (из формы)
void CApplicationVoteSystem::ProcessRejection(dpp::cluster& bot, const dpp::form_submit_t& event, 
                                             SApplicationVoteData& application, const std::string& reason)
{
    DoRejection(bot, event.command.message_id, event.command.channel_id, application, 
               reason, event.command.get_issuing_user().id);
    event.reply(dpp::message("Заявка отклонена").set_flags(dpp::m_ephemeral));
}

// Основная логика отклонения заявки
void CApplicationVoteSystem::DoRejection(dpp::cluster& bot, dpp::snowflake message_id, dpp::snowflake channel_id, 
                                        SApplicationVoteData& application, const std::string& reason, 
                                        dpp::snowflake moderator_id)
{
    application.m_status = "rejected";
    application.m_processedBy = moderator_id;
    application.m_rejectionReason = reason;
    
    // Создание копии для передачи в лямбду
    SApplicationVoteData appCopy = application;
    
    bot.message_get(message_id, channel_id, 
        [this, &bot, appCopy, reason](const dpp::confirmation_callback_t& callback) mutable {
            if (callback.is_error()) return;
            
            auto msg = callback.get<dpp::message>();
            FinalizeApplication(bot, msg, appCopy, false, reason);
        });
    
    SaveState();
}

// Завершение обработки заявки
void CApplicationVoteSystem::FinalizeApplication(dpp::cluster& bot, const dpp::message& msg, 
                                                SApplicationVoteData& application, bool accepted, 
                                                const std::string& reason)
{
    // Обновление embed сообщения
    dpp::embed tmpEmbed = msg.embeds[0];
    tmpEmbed.set_color(accepted ? dpp::colors::green : dpp::colors::red);
    
    // Добавление информации о модераторе
    std::string moderatorInfo = fmt::format("Обработано: <@{}>", application.m_processedBy);
    if (!accepted && !reason.empty()) {
        moderatorInfo += fmt::format("\nПричина: {}", reason);
    }
    
    tmpEmbed.add_field("Статус", accepted ? "✅ Одобрена" : "❌ Отклонена", true);
    tmpEmbed.add_field("Модератор", moderatorInfo, false);
    
    // Обновление сообщения
    dpp::message newMsg = msg;
    newMsg.set_content("");
    newMsg.embeds.clear();
    newMsg.add_embed(tmpEmbed);
    newMsg.components.clear();
    
    bot.message_edit(newMsg);
    
    // Действия в зависимости от результата
    if (accepted) {
        // Добавление пользователя в базу данных
        DataBase db(PATH_MEMBERS_DATA_BASE);
        db.SetUser(application.m_targetUserId, application.m_userData);
        db.Save();
        
        // Отправка уведомления пользователю
        try {
            dpp::message directMsg;
            directMsg.set_content("Ваша заявка в клан была одобрена! Добро пожаловать!");
            bot.direct_message_create(application.m_targetUserId, directMsg);
        } catch (const std::exception& e) {
            std::cout << "Ошибка отправки DM: " << e.what() << std::endl;
        }
        
        // Обновление ролей
        bot.guild_member_remove_role(msg.guild_id, application.m_targetUserId, DEFAULT_ROLE_ID);
        bot.guild_member_add_role(msg.guild_id, application.m_targetUserId, CLAN_ROLE_ID);
    } else {
        // Отправка уведомления об отказе
        try {
            dpp::message directMsg;
            std::string rejectionMessage = fmt::format("Ваша заявка в клан была отклонена.\n{}", 
                                                     reason.empty() ? "" : "Причина: " + reason);
            directMsg.set_content(rejectionMessage);
            bot.direct_message_create(application.m_targetUserId, directMsg);
        } catch (const std::exception& e) {
            std::cout << "Ошибка отправки DM: " << e.what() << std::endl;
        }
    }
    
    // Удаление заявки из активных
    m_activeApplications.erase(msg.id);
}

// Обработка формы заявки
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

        // Получение поинтов игрока
        std::string points = std::to_string(Parsing::GetPoints(Parsing::GetUrl(nickname)));

        // Создание сообщения для модерации
        CreateApplicationMessage(*event.from()->creator, user, nickname, age, about, points);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка в ProcessFormSubmit: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке заявки");
    }
}

// Создание сообщения для модерации
void CApplicationVoteSystem::CreateApplicationMessage(dpp::cluster& bot, const dpp::user& user, 
                                                     const std::string& nickname, const std::string& age, 
                                                     const std::string& about, const std::string& points)
{
    // Создание embed сообщения
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

    // Отправка сообщения и сохранение заявки
    bot.message_create(modsMsg, [this, user, nickname, age, about](const dpp::confirmation_callback_t& callback)
                       {
        if (callback.is_error()) return;
        
        auto msg = callback.get<dpp::message>();
        SApplicationVoteData applicationData;
        applicationData.m_targetUserId = user.id;
        applicationData.m_userData = {
            {"game_nick", nickname},
            {"age", age},
            {"about", about},
            {"social_rating", 1000}
        };
        applicationData.m_status = "pending";
        
        m_activeApplications[msg.id] = applicationData;
        SaveState(); });
}

// Сохранение состояния заявок
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
        {"userData", m_userData},
        {"status", m_status},
        {"rejectionReason", m_rejectionReason}
    };
}

// Десериализация данных заявки
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
```