#include "support.h"
#include <bot_core.h>
#include <cpr/cpr.h>
struct Fixture {
 Sandbox sandbox;dpp::cluster bot;std::unique_ptr<CBotCore> core;
 Fixture(){dpp::users.clear();cpr::requests=0;cpr::handler={};core=std::make_unique<CBotCore>(&bot);}
 ~Fixture(){core.reset();cpr::handler={};dpp::users.clear();}
 void init(){bot.ready({});}
 void moderators(int n){for(int i=1;i<=n;++i){dpp::guild_member m;m.user_id=i;m.roles={core->Config()->MODER_ROLE_ID};bot.members[i]=m;}init();}
};
TEST_CASE("integration.core.initialization_and_commands") {
 Fixture f;CHECK(f.core->Bot()==&f.bot);REQUIRE(f.core->Config());REQUIRE(f.core->Console());REQUIRE(f.core->DataBase());f.init();
 CHECK(f.bot.commands==std::vector<std::string>{"apply","profile","join"});auto commands=f.core->Console()->GetAllCommands();CHECK(std::find(commands.begin(),commands.end(),"exec")!=commands.end());
 write_file("script","role_id 123");line(*f.core->Console(),"exec script");CHECK(f.core->Config()->ROLE_ID==123);
 CHECK(f.core->m_ClanMemberManager.Name()=="clan_member_manager");CHECK(f.core->m_FindTas.Name()=="find_tas");
}
TEST_CASE("integration.members.events_and_moderators") {
 Fixture f;f.moderators(2);auto& m=f.core->m_ClanMemberManager;CHECK(m.NumModers()==2);CHECK(m.IsModerator(dpp::snowflake(1)));CHECK_FALSE(m.IsModerator(dpp::snowflake(99)));
 dpp::guild_member_update_t e;e.updated.user_id=1;f.bot.member_update(e);CHECK(m.NumModers()==1);f.bot.member_update(e);CHECK(m.NumModers()==1);
 e.updated.roles={f.core->Config()->MODER_ROLE_ID};f.bot.member_update(e);f.bot.member_update(e);CHECK(m.NumModers()==2);
 dpp::guild_member_add_t a;a.added.guild_id=77;a.added.user_id=55;f.bot.member_add(a);REQUIRE(f.bot.added_roles.size()==1);CHECK(uint64_t(std::get<1>(f.bot.added_roles[0]))==55);
 f.bot.fail=true;f.init();CHECK(m.NumModers()==2);
}
TEST_CASE("integration.members.add_clan_member") {
 Fixture f;auto u=sample();f.core->m_ClanMemberManager.AddClanMember(u);
 REQUIRE(f.bot.added_roles.size()==1);REQUIRE(f.bot.removed_roles.size()==1);REQUIRE(f.bot.edited_members.size()==1);CHECK(f.bot.edited_members[0].nickname==u.m_GameNick);
 auto stored=f.core->DataBase()->Load<SUserData>("clan_members",42);REQUIRE(stored);CHECK(stored->m_Clan==f.core->Config()->ClanTag);
}
TEST_CASE("integration.members.api_errors") {
 Fixture f;f.bot.fail=true;f.core->m_ClanMemberManager.AddClanMember(sample());CHECK(f.bot.edited_members.empty());CHECK(f.bot.added_roles.size()==1);
}
TEST_CASE("unit.apply.uninitialized_init") {CApplyCommand c;CHECK_NOTHROW(c.OnInit());}
TEST_CASE("integration.apply.message_modal_dispatch") {
 Fixture f;dpp::slashcommand_t e;e.command.name="apply";f.bot.slash(e);REQUIRE(e.replies.size()==1);CHECK_FALSE(e.replies[0].content.empty());REQUIRE(e.replies[0].components.size()==1);CHECK(e.replies[0].components[0].components[0].custom_id=="apply_button");
 dpp::button_click_t b;b.custom_id="apply_button";f.bot.button(b);REQUIRE(b.dialogs.size()==1);auto modal=b.dialogs[0];CHECK(modal.custom_id=="apply_vote_manager_form");REQUIRE(modal.components.size()==3);CHECK(modal.components[0].max_length==32);CHECK(modal.components[1].min_length==5);CHECK(modal.components[1].required);
 line(*f.core->Console(),"apply");
}
TEST_CASE("integration.profile.slash_found_missing_self") {
 Fixture f;auto u=sample();f.core->DataBase()->Save("clan_members",42,u);
 dpp::slashcommand_t e;e.command.name="profile";e.command.usr.id=42;f.bot.slash(e);REQUIRE(e.replies.size()==1);REQUIRE(e.replies[0].embeds.size()==1);auto embed=e.replies[0].embeds[0];CHECK(embed.title=="👤 "+u.m_GameNick);CHECK(embed.description==u.m_About);CHECK(embed.fields.size()==3);CHECK(embed.fields[1].value=="29.02");
 e.parameters["user"]=dpp::snowflake(99);f.bot.slash(e);CHECK(e.replies.back().content=="User not found");CHECK(e.replies.back().flags==dpp::m_ephemeral);
 e.parameters["user"]=dpp::snowflake(0);f.bot.slash(e);CHECK(e.replies.back().content=="User not found");
}
TEST_CASE("unit.profile.render_optional_fields") {
 CProfileCommand p;auto u=sample();u.m_About="";u.m_Clan="";u.m_Age.reset();auto embed=p.GenerateEmbend(u);CHECK(embed.description=="_Информация не указана_");REQUIRE(embed.fields.size()==1);CHECK(embed.fields[0].value=="29.02");
 Capture c;p.PrintProfileConsole(u);CHECK(c.out.str().find("None")!=std::string::npos);CHECK(c.out.str().find("AGE:")==std::string::npos);CHECK(c.out.str().find("29.02")!=std::string::npos);
}
TEST_CASE("integration.profile.console_and_wrong_event") {
 Fixture f;auto u=sample();f.core->DataBase()->Save("clan_members",42,u);Capture c;line(*f.core->Console(),"profile; profile bad 99 42");CHECK(c.out.str().find("PROFILE")!=std::string::npos);CHECK(c.out.str().find("Wrong User")!=std::string::npos);
 std::string n="profile";CConsole::IResult r(n);dpp::button_click_t event;r.m_Event=&event;r.m_Flags=SLASH_COMMAND;f.core->m_CommandHandler.m_Profile.Execute(r);REQUIRE(event.replies.size()==1);CHECK(event.replies[0].content=="Ooops... error");
}
TEST_CASE("unit.profile.target_user") {
 CProfileCommand p;dpp::slashcommand_t e;e.command.usr.id=12;e.parameters["user"]=dpp::snowflake(0);CHECK(uint64_t(p.GetTargetUser(&e))==12);e.parameters["user"]=dpp::snowflake(9);CHECK(uint64_t(p.GetTargetUser(&e))==9);
}
TEST_CASE("unit.findtas.parse_cache_and_errors") {
 CFindTas f;cpr::requests=0;cpr::handler=[](auto& u,auto& p){CHECK(u.str=="https://ddnet.org/players/");CHECK(p[0].second=="nick");return cpr::Response{200,R"({"deleted_ranks":[{"map":"Map","country":"GER"},{}]})"};};
 auto p=f.Player("nick");CHECK(p.m_Nick=="nick");REQUIRE(p.m_Finishes.size()==2);CHECK(p.m_Finishes[0].m_Name=="Map");CHECK(p.m_Finishes[0].m_Server=="GER");CHECK_FALSE(p.m_Finishes[0].m_IsTeamRank);CHECK(p.m_Finishes[0].m_Rank==0);CHECK(p.m_Finishes[1].m_Name.empty());f.Player("nick");CHECK(cpr::requests==1);
 cpr::handler=[](auto&,auto&){return cpr::Response{200,"{bad"};};CHECK_THROWS_AS(f.Player("bad"),nlohmann::json::parse_error);
 cpr::handler=[](auto&,auto&){return cpr::Response{200,R"({"deleted_ranks":[{"map":7}]})"};};CHECK_NOTHROW(f.Player("wrongtype"));
 cpr::handler={};CHECK(f.Player("empty").m_Finishes.empty());
}
TEST_CASE("unit.votes.birthday_valid_and_invalid") {
 CApplyVoteManager m;
 for(auto [input,year]:std::vector<std::pair<std::string,int>>{{"01.01.2000",2000},{"31.12.25",2025},{"28.02.26",1926},{"01.01.99",1999},{"01.01.00",2000}}){auto d=m.ParseBirthday(input);REQUIRE(d);CHECK(d->m_Year==year);}
 auto noyear=m.ParseBirthday("25.12");REQUIRE(noyear);CHECK(noyear->m_Day==25);CHECK(noyear->m_Month==12);CHECK_FALSE(noyear->m_Year);
 for(auto s:{"","hello","32.01","01.13","00.00"})CHECK_FALSE(m.ParseBirthday(s));
}
TEST_CASE("unit.votes.age_boundaries") {
 CApplyVoteManager m;CHECK_FALSE(m.CalculateAge({1,1,{}}));auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());auto tm=*std::localtime(&now);int year=tm.tm_year+1900;
 CHECK(m.CalculateAge({tm.tm_mday,tm.tm_mon+1,year-20})==20);CHECK(m.CalculateAge({1,1,year-20})==20);
 if(tm.tm_mon!=11||tm.tm_mday!=31)CHECK(m.CalculateAge({31,12,year-20})==19);
}
TEST_CASE("integration.votes.form_validation") {
 Fixture f;std::string n="form";CConsole::IResult r(n);dpp::form_submit_t e;r.m_Event=&e;
 CHECK_NOTHROW(f.core->m_ClanVoteManager.FormSubmit(r));r.m_Args={"nick","invalid","about"};r.m_Flags=MODAL;f.core->m_ClanVoteManager.FormSubmit(r);REQUIRE(e.replies.size()==1);CHECK(e.replies[0].content=="Неверная дата");CHECK_FALSE(e.thought);
}
TEST_CASE("integration.votes.form_and_restore") {
 Fixture f;dpp::users[42]={42,"test"};dpp::form_submit_t event;event.custom_id="apply_vote_manager_form";event.command.usr.id=42;
 for(auto v:{"nick","01.01.2000","about"}){dpp::component c;c.value=std::string(v);event.components.push_back(c);}f.bot.form(event);CHECK(event.thought);CHECK(event.deleted);REQUIRE(f.core->m_ClanVoteManager.m_vpVotes.size()==1);auto& vote=*f.core->m_ClanVoteManager.m_vpVotes.begin()->second;CHECK(vote.GetUser().m_GameNick=="nick");CHECK(vote.Id()==1);REQUIRE(f.bot.edited.size()==1);CHECK(f.bot.edited[0].components.size()==2);
 CApplyVoteManager restored;restored.OnModuleInit(f.core.get());REQUIRE(restored.m_vpVotes.size()==1);CHECK(restored.m_vpVotes.at(1)->GetUser().m_Id==42);
}
TEST_CASE("integration.votes.duplicate_change_and_finale") {
 Fixture f;f.moderators(3);dpp::users[42]={42,"test"};CApplyVoteManager::CClanVote vote(sample());vote.OnModuleInit(f.core.get());vote.SetId(7);vote.StartVote();bool done=false;vote.OnFinale([&]{done=true;});
 vote.AddVote(1,EVoteOptions::YES);vote.AddVote(1,EVoteOptions::YES);CHECK(vote.m_Yes==1);CHECK(vote.m_No==0);vote.AddVote(1,EVoteOptions::NO);CHECK(vote.m_Yes==0);CHECK(vote.m_No==1);vote.AddVote(1,EVoteOptions::YES);vote.AddVote(2,EVoteOptions::YES);CHECK_FALSE(done);vote.AddVote(3,EVoteOptions::NO);CHECK(done);CHECK(f.bot.dms.size()==1);REQUIRE(f.core->DataBase()->Load<SUserData>("clan_members",42));
}
TEST_CASE("integration.votes.tie_declines") {
 Fixture f;f.moderators(2);dpp::users[42]={42,"test"};CApplyVoteManager::CClanVote v(sample());v.OnModuleInit(f.core.get());v.SetId(1);v.StartVote();bool done=false;v.OnFinale([&]{done=true;});v.AddVote(1,EVoteOptions::YES);v.AddVote(2,EVoteOptions::NO);CHECK(done);REQUIRE(f.bot.dms.size()==1);CHECK(f.bot.dms[0].content==f.core->Config()->DIRECT_MESSAGE_REJECT);CHECK_FALSE(f.core->DataBase()->Load<SUserData>("clan_members",42));
}
TEST_CASE("integration.votes.button_permissions_missing_vote") {
 Fixture f;f.moderators(1);dpp::button_click_t e;e.custom_id="apply_vote_manager_button 999 yes";e.command.usr.id=99;f.bot.button(e);REQUIRE(e.replies.size()==1);CHECK(e.replies[0].flags==dpp::m_ephemeral);e.command.usr.id=1;f.bot.button(e);CHECK(e.replies.size()==1);
}
TEST_CASE("unit.votes.render_and_api_failure") {
 Fixture f;dpp::users[42]={42,"test"};CApplyVoteManager::CClanVote v(sample());v.OnModuleInit(f.core.get());v.SetId(8);v.StartVote();auto msg=v.GenerateMessage();REQUIRE(msg.components.size()==2);CHECK(msg.components[1].components[0].custom_id=="apply_vote_manager_button 8 yes");CHECK(msg.flags==dpp::m_using_components_v2);
 f.bot.fail=true;CHECK_NOTHROW(v.SyncMessage());CHECK(f.bot.created.size()==1);dpp::users.clear();CHECK_THROWS_AS(v.GenerateMessage(),std::runtime_error);
}
TEST_CASE("integration.music.register_and_ignore_non_slash") {
 Fixture f;f.init();std::string n="join";CConsole::IResult r(n);dpp::button_click_t e;r.m_Event=&e;CHECK_NOTHROW(f.core->m_Music.Test(r));
}
TEST_CASE("unit.votes.render_long_deleted_rank_list") {
 Fixture f;dpp::users[42]={42,"test"};CFindTas::CPlayer p;for(int i=0;i<8;++i)p.m_Finishes.push_back({"Map"+std::to_string(i),"GER",false,0});f.core->m_FindTas.m_CachePlayer[sample().m_GameNick]=p;
 CApplyVoteManager::CClanVote v(sample());v.OnModuleInit(f.core.get());v.SetId(1);v.StartVote();dpp::component box;v.AddDeletedRanks(&box);REQUIRE(box.components.size()==1);CHECK(box.components[0].content.find("Map4")!=std::string::npos);CHECK(box.components[0].content.find("Map5")==std::string::npos);CHECK(box.components[0].content.find("ещё 3")!=std::string::npos);
 auto u=sample();u.m_Age.reset();CApplyVoteManager::CClanVote younger(u);younger.OnModuleInit(f.core.get());younger.SetId(2);younger.StartVote();
}
TEST_CASE("integration.votes.button_record_and_change") {
 Fixture f;f.moderators(3);dpp::users[42]={42,"test"};auto v=std::make_unique<CApplyVoteManager::CClanVote>(sample());v->OnModuleInit(f.core.get());v->SetId(1);v->StartVote();f.core->m_ClanVoteManager.m_vpVotes[1]=std::move(v);
 dpp::button_click_t e;e.command.usr.id=1;e.custom_id="apply_vote_manager_button 1 yes";f.bot.button(e);CHECK(f.core->m_ClanVoteManager.m_vpVotes.at(1)->m_Yes==1);CHECK(e.replies.size()==1);e.custom_id="apply_vote_manager_button 1 no";f.bot.button(e);CHECK(f.core->m_ClanVoteManager.m_vpVotes.at(1)->m_No==1);e.custom_id="apply_vote_manager_button 1 invalid";f.bot.button(e);CHECK(f.core->m_ClanVoteManager.m_vpVotes.at(1)->m_No==1);
 line(*f.core->Console(),"update_vote");std::string n="bad";CConsole::IResult r(n);CHECK_NOTHROW(f.core->m_ClanVoteManager.ButtonClick(r));
}
TEST_CASE("unit.modules.names_and_empty_hooks") {CCommandHandler h;CHECK(h.Name()=="CCommandHandler");CMemberManager m;CHECK(m.Name()=="member_manager");CFindTas t;t.OnInit();t.OnConsoleInit();CApplyVoteManager v;v.OnInit();CHECK(v.IVoteManager::Name()=="vote_manager");CApplyVoteManager::CClanVote vote;vote.OnInit();vote.OnConsoleInit();CHECK(vote.Name()=="vote");CProfileCommand p;CHECK(p.ICommand::Name()=="icommand");CHECK(p.IModule::Name()=="IModule");}
