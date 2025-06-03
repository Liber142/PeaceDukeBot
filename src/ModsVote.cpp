#include "ModsVote.h"
#include "ConstAgr.h"

#include <dpp/colors.h>
#include <dpp/message.h>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <string>

std::unordered_map<dpp::snowflake, VoteData> ModsVote::activeVotes;

void ModsVote::Initialize(dpp::cluster& bot, DataBase* v_db, DataBase* m_db, std::string AplicationAceptedMessage, std::string AplicationRejectedMessage) 
{
    voteDatabase = v_db;
    std::cout << "db: " << v_db->GetFilePath() << std::endl;
    std::cout << "voteDatabase: " << voteDatabase->GetFilePath() << std::endl;
    std::cout << "db: " << v_db->p_GetFilePath() << std::endl;
    std::cout << "voteDatabase: " << voteDatabase->p_GetFilePath() << std::endl;
    LoadActiveVotes(); 

    bot.on_button_click([m_db, AplicationAceptedMessage, AplicationRejectedMessage, &bot](const dpp::button_click_t& event) {
        if (event.custom_id == "accept" || event.custom_id == "reject") 
        {
            auto it = activeVotes.find(event.command.message_id);
            if (it == activeVotes.end()) 
            {
                event.reply(dpp::message("Голосование не найдено или завершено").set_flags(dpp::m_ephemeral));
                return;
            }

            VoteData& vote = it->second;
            dpp::user user = event.command.get_issuing_user();
            dpp::snowflake userId = user.id;
            std::cout << "Click from: " << user.username << std::endl;

            if (!vote.votedUsers.count(userId)) 
            {
                std::cout << "Vote accept for this user" << std::endl;
                (event.custom_id == "accept") ? vote.voteAccept++ : vote.voteReject++;
           		vote.votedUsers.insert(userId);

        		event.from()->creator->message_get(event.command.message_id, event.command.channel_id, 
            	[event, &vote, userId, m_db, AplicationAceptedMessage, AplicationRejectedMessage, &bot](const dpp::confirmation_callback_t& callback) 
            	{
                    bool voteResult = (vote.voteAccept > vote.voteReject);
               		if (callback.is_error()) return;

                    auto msg = callback.get<dpp::message>();
                    if ((vote.voteAccept + vote.voteReject) >= 3)
                    {
                   		msg.set_content("");

                        dpp::embed tmpEmbed = msg.embeds[0];
                        tmpEmbed.set_color((voteResult) ? dpp::colors::green : dpp::colors::red);
                        activeVotes[msg.id].user["clan"] = voteResult ? "Peace Duke" : nullptr;
                        msg.embeds.clear();
                        msg.add_embed(tmpEmbed);

                        msg.components.clear();

                        nlohmann::json newClanMember = activeVotes[msg.id].user;
                        std::cout << "activeVotes[msg.id].user = " << to_string(activeVotes[msg.id].user) << std::endl
                                  << "\tactiveVotes[msg.id].user[\"game_nick\"] = " << to_string(activeVotes[msg.id].user["game_nick"]) << std::endl
                                  << "\tactiveVotes[msg.id].user[\"age\"] = " << to_string(activeVotes[msg.id].user["age"]) << std::endl
                                  << "\tactiveVotes[msg.id].user[\"about\"] = " << to_string(activeVotes[msg.id].user["about"]) << std::endl;

                        m_db->SetUser(activeVotes[msg.id].targedUserId, newClanMember);
                        m_db->Save();

                        dpp::message directMsg;
                        directMsg.set_content(voteResult ? AplicationAceptedMessage : AplicationRejectedMessage);
                        try
                        {
                            bot.direct_message_create(activeVotes[msg.id].targedUserId, directMsg);   
                        }
                        catch (const std::exception& e)
                        {
                            std::cout << "ERROR in send DM" << e.what() << std::endl;
                        }
                        std::cout << "activeVotes[msg.id].targedUserId: " << activeVotes[msg.id].targedUserId << std::endl;
                        activeVotes.erase(msg.id);
                    }
                    else
                    {
                        msg.set_content(fmt::format("**Голосование:**\n✅ За: {}\n❌ Против: {}", vote.voteAccept, vote.voteReject));
                        std::cout << "Vote result: " << vote.voteAccept << " | " << vote.voteReject << std::endl;                        
                    }
               		
                	event.from()->creator->message_edit(msg);
           		});
            }
            event.reply();
            SaveActiveVotes();
        }
    });
}

void ModsVote::RegisterVote(dpp::cluster& bot, const dpp::form_submit_t& event) {
    try 
    {
        dpp::user user = event.command.get_issuing_user();
        std::string nickname = std::get<std::string>(event.components[0].components[0].value);
        std::string age = std::get<std::string>(event.components[1].components[0].value);
        std::string about = std::get<std::string>(event.components[2].components[0].value);

        // Проверка возраста
        if (!std::all_of(age.begin(), age.end(), ::isdigit)) 
        {
            event.reply(dpp::message("Еблан возраст цифрами пиши").set_flags(dpp::m_ephemeral));
            return;
        }

        int i_age = std::stoi(age);
        if (i_age < 1) 
        {
            event.reply(dpp::message("Совсем конченый? Что с возрастом?").set_flags(dpp::m_ephemeral));
            return;
        }

        dpp::embed embed = dpp::embed()
            .set_author(user.username, "", user.get_avatar_url())
            .set_color(dpp::colors::red)
            .set_color(dpp::colors::sti_blue)
            .set_title("Новая заявка")
            .add_field("Ник: ", nickname)
            .add_field("Возраст: ", age)
            .add_field("О себе", about);

        dpp::message modsMsg(CHANNEL_MODERATION_ID, embed);
        dpp::component actionRow;
        actionRow.add_component(
            dpp::component()
                .set_label("Принять")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_success)
                .set_id("accept")
            );
        actionRow.add_component(
            dpp::component()
                .set_label("Отклонить")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_danger)
                .set_id("reject")
            );
        modsMsg.add_component(actionRow);

        bot.message_create(modsMsg, [user, nickname, age, about](const dpp::confirmation_callback_t& callback) 
        {
            if (callback.is_error()) return;
            auto msg = callback.get<dpp::message>();
            activeVotes[msg.id] = VoteData();
            activeVotes[msg.id].targedUserId = user.id;
            activeVotes[msg.id].user = {
                {"game_nick", nickname},
                {"age", age},
                {"about", about},
                {"social_rating", 1000}
            };
            std::cout << "activeVotes[msg.id].user =" << nickname << "|" << age << "|" << about << std::endl;
            activeVotes[msg.id].to_json();
            SaveActiveVotes();
        });

        event.reply(dpp::message("Заявка отправлена").set_flags(dpp::m_ephemeral));
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Ошибка в RegisterVote: " << e.what() << std::endl;
        event.reply("Произошла ошибка при обработке заявки");
    }
}


void ModsVote::LoadActiveVotes()
{
    std::cout << "ModsVote::LoadActiveVotes()" << std::endl;
    nlohmann::json data = voteDatabase->GetVoteData();
    if (!data.is_null())
    {
        activeVotes.clear();
        for (auto& [key, value] : data.items())
        {
            dpp::snowflake msgId = std::stoull(key);
            activeVotes[msgId] = VoteData::from_json(value);
            std::cout << "Loaded vote data for " << key << ": " << value.dump(2) << std::endl;
        }
    }
}

void ModsVote::SaveActiveVotes()
{
    //std::cout << "ModsVote::SaveActiveVotes(" << voteDatabase->GetFilePath() << ")" << std::endl;
    nlohmann::json data = voteDatabase->GetVoteData();
    for (auto& [msgId, vote] : activeVotes)
    {
        data[std::to_string(msgId)] = vote.to_json();
    }
    voteDatabase->SaveVoteData(data);
}