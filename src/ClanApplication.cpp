#include <dpp/dispatcher.h>
#include "ClanApplication.h"

void ClanApplication::ShowApplicationModal(const dpp::button_click_t& event)
{
    dpp::interaction_modal_response modal("clan_apply", "Заявка в клан");

    modal.add_component(
        dpp::component()
            .set_label("Ваш игровой ник")
            .set_id("gameNick")
            .set_type(dpp::cot_text)
            .set_placeholder("Введите ваш никнейм")
            .set_max_length(32)
            .set_required(true)
            .set_text_style(dpp::text_short));

    modal.add_row();
    modal.add_component(
        dpp::component()
            .set_label("Ваш возраст")
            .set_id("age")
            .set_type(dpp::cot_text)
            .set_placeholder("Укажите ваш возраст")
            .set_required(true)
            .set_max_length(2)
            .set_min_length(1)
            .set_text_style(dpp::text_short));

    modal.add_row();
    modal.add_component(
        dpp::component()
            .set_label("Расскажите о себе")
            .set_id("about")
            .set_type(dpp::cot_text)
            .set_placeholder("Ваши навыки, опыт игры и т.д.")
            .set_text_style(dpp::text_paragraph));

    event.dialog(modal);
}
