#include "engine/config.h"
#include "engine/database.h"

#include <iostream>

#include <dpp/dpp.h>

class CommandHandler 
{
public: 
    CommandHandler(IDataBase* db) {};
};

int main()
{
    char* token = std::getenv("DISCORD_TOKEN");
    if(!token)
    {
        std::cerr << "Missing env DISCORD_TOKEN" << std::endl; 
        return 1;
    }

    dpp::cluster bot(token);
    JsonDataBase db;
    db.Connect("members");


    CommandHandler command(&db);
    bot.on_ready([&bot](const dpp::ready_t& event) {
        std::cout << "I'm " << bot.me.format_username() << " ready" << std::endl;

     });

    bot.on_message_create([&bot, &db](const dpp::message_create_t& event) {
        UserData user;
        const dpp::user author = event.msg.author;
        user.id = author.id;
        user.age = 12;
        user.socialRating = 100;
        user.about = g_Config.about;
        user.clan = "Hui";
        user.gameNick = author.global_name;

        db.AddUser(user.id, user);
    });

    bot.start();
}
