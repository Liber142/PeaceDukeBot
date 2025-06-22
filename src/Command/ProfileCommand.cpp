#include <dpp/appcommand.h>
#include <dpp/cache.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

#include "../../include/Commands/ProfileCommand.h"
#include "../../include/DataBase.h"
#include "../../include/Parsing.h"

ProfileCommand::ProfileCommand(dpp::cluster& bot_instance, DataBase db_instance) : bot(bot_instance), db(db_instance) {};

void ProfileCommand::Execute(const dpp::slashcommand_t& event)
{
    dpp::user target_user = GetTargetUser(event); 
    if (target_user.id == 0) 
    {
        event.reply("Invalid user.");
        return;
    }
    std::cout << "target_user: " << target_user.username << "using /Profile" << std::endl;


    nlohmann::json j_user = db.GetUser(target_user.id);
    if (j_user.is_null() || j_user.empty()) 
    {
        std::cerr << "User data is null or empty." << std::endl;
        event.reply("User data not found or invalid.");
        return;
    }
    
/*    "1133635252691161158": {
        "about": "Играю как лох потомучто лагает",
        "age": "14",
        "clan": "Peace Duke",
        "game_nick": "sladorc",
        "social_rating": 1000

*/
    std::string nick = j_user.value("game_nick", "");
    std::cout << "nick: " << nick << std::endl;
    std::string age = j_user.value("age", "");
    std::cout << "age: " << age << std::endl;
    std::string about = j_user.value("about", "");
    std::cout << "about: " << about << std::endl;
    std::string social_rating = std::to_string(j_user.value("social_rating", 0));
    std::cout << "social_rating: " << social_rating << std::endl;
    std::string clan = j_user.value("clan", "");
    std::cout << "clan: " << clan << std::endl;


    dpp::embed embed = dpp::embed()
        .set_author(target_user.username, "", target_user.get_avatar_url())
        .set_color(dpp::colors::aqua)
        .set_title("Профиль: " + nick)
        .add_field("Возраст: ", age)
        .add_field("Клан", clan)
        .add_field("Социальный рейтинг: ", social_rating)
        .add_field("Поинты: ", "---")
        .add_field("О себе: ", about);

    dpp::message msg = dpp::message().add_embed(embed);

    event.reply(msg);

    std::cout << "Ну вот щас начинает парсить" << std::endl;
    int points = Parsing::GetPoints(Parsing::GetUrl(j_user.value("game_nick", "")));
    std::cout << "Ну щас он получил поинты: " << points << std::endl;

    embed = dpp::embed()
        .set_author(target_user.username, "", target_user.get_avatar_url())
        .set_color(dpp::colors::aqua)
        .set_title("Профиль: " + nick)
        .add_field("Возраст: ", age)
        .add_field("Клан", clan)
        .add_field("Социальный рейтинг: ", social_rating)
        .add_field("Поинты: ", std::to_string(points))
        .add_field("О себе: ", about);

    msg.embeds.clear();
    msg.add_embed(embed);
    event.edit_response(msg);
}

dpp::slashcommand ProfileCommand::Register()
{
	  dpp::slashcommand cmd("profile", "Покажет профиль участника", bot.me.id);

      cmd.add_option(dpp::command_option(dpp::co_user, "user", "Чей профиль показать?", false));
    return cmd;
}

dpp::user ProfileCommand::GetTargetUser(const dpp::slashcommand_t& event) 
{
    if (auto param = event.get_parameter("user"); 
        std::holds_alternative<dpp::snowflake>(param)) {
        return *dpp::find_user(std::get<dpp::snowflake>(param));
    }
    return event.command.get_issuing_user();
}
