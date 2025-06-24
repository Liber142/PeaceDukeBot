
#include "../../include/Events/RegisterEvent.h"

#include "../../include/ConstAgr.h"

#include <dpp/appcommand.h>
#include <dpp/cache.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/guild.h>
#include <dpp/permissions.h>
#include <filesystem>

void RegisterEvent::Register(dpp::cluster& bot)
{
	// Обработчик кнопки start_event
bot.on_button_click([&bot](const dpp::button_click_t& event) {
    if (event.custom_id == "start_event") 
    {
        // Первый шаг - выбор ивентовой роли
        dpp::message msg;
        msg.set_content("Выберите ивенторую роль:");
        msg.add_component(
            dpp::component().add_component(
                dpp::component()
                .set_type(dpp::cot_role_selectmenu)
                .set_id("select_event_role")
            )
        );
        msg.set_flags(dpp::m_ephemeral);
        event.reply(dpp::ir_channel_message_with_source, msg);
    }
});

// Обработчик выбора ивентовой роли
bot.on_select_click([&bot](const dpp::select_click_t& event) 
{
    if (event.custom_id == "select_event_role") 
    {
        // Второй шаг - выбор роли-критерия
        dpp::message msg;
        msg.set_content("Выберите роль-критерий для участников:");
        msg.add_component(
            dpp::component().add_component(
                dpp::component()
                .set_type(dpp::cot_role_selectmenu)
                .set_id("select_criteria_role")
            )
        );
        msg.set_flags(dpp::m_ephemeral);
        event.reply(dpp::ir_channel_message_with_source, msg);
    }
    else if (event.custom_id == "select_criteria_role") 
    {
        // Третий шаг - модальное окно с описанием и временем
        dpp::interaction_modal_response modal("event_modal", "Детали ивента");
        
        // Поле для времени
        modal.add_component(
            dpp::component()
            .set_label("Время проведения")
            .set_id("event_time")
            .set_type(dpp::cot_text)
            .set_placeholder("Например: 25 декабря в 20:00")
            .set_min_length(5)
            .set_max_length(50)
            .set_text_style(dpp::text_short)
        );
        modal.add_row();
        
        // Поле для описания
        modal.add_component(
            dpp::component()
            .set_label("Описание ивента")
            .set_id("event_description")
            .set_type(dpp::cot_text)
            .set_placeholder("Подробное описание ивента...")
            .set_min_length(10)
            .set_max_length(2000)
            .set_text_style(dpp::text_paragraph)
        );
        
        event.dialog(modal);
    }
});
}