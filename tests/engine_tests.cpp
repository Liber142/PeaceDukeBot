#include "support.h"
#include <limits>
TEST_CASE("unit.console.registration") {
 CConsole c;int calls=0;
 c.Register("a",{"s"},BUTTON,[&](auto&){++calls;},"help");
 c.Register("a",{},BUTTON,[&](auto&){calls+=100;},"ignored");
 CHECK(c.GetAllCommands()==std::vector<std::string>{"a"});
 REQUIRE(c.FindCommand("a",BUTTON));CHECK(c.FindCommand("a",BUTTON)->m_Help=="help");
 CHECK(c.FindCommand("a",MODAL)==nullptr);CHECK(c.FindCommand("missing")==nullptr);
 line(c,"a");CHECK(calls==1);
 c.Register("a",{},MODAL,[](auto&){},"");CHECK(c.GetAllCommands().size()==2);
}
TEST_CASE("unit.console.parser") {
 CConsole c;std::vector<std::vector<std::string>> seen;
 c.Register("x",{},0,[&](auto& r){CHECK(r.m_Name=="x");seen.push_back(r.m_Args);},"");
 SUBCASE("whitespace"){line(c," \tx  one\t two  ");REQUIRE(seen.size()==1);CHECK(seen[0]==std::vector<std::string>{"one","two"});}
 SUBCASE("multiple"){line(c,"mc;x 1;; x 2 ; x 3 # comment");REQUIRE(seen.size()==3);CHECK(seen[2][0]=="3");}
 SUBCASE("quoted"){line(c,"x \"hello ; # world\" a\\ b \"escaped \\\" quote\" \\\\");REQUIRE(seen.size()==1);CHECK(seen[0]==std::vector<std::string>{"hello ; # world","a b","escaped \" quote","\\"});}
 SUBCASE("empty and comments"){for(auto s:{"","  ",";;;","# x","  # comment"})line(c,s);CHECK(seen.empty());}
 SUBCASE("unknown"){Capture cap;line(c,"unknown");CHECK(cap.err.str().find("Command not found")!=std::string::npos);}
 SUBCASE("unclosed quote"){line(c,"x \"hello");REQUIRE(seen.size()==1);CHECK(seen[0][0]=="hello");}
}
TEST_CASE("unit.console.integer_conversion") {
 std::string name="x";CConsole::IResult r(name);
 for(auto [s,v]:std::vector<std::pair<std::string,int>>{{"0",0},{"-1",-1},{"2147483647",2147483647},{"-2147483648",(-2147483647-1)},{"bad",-1},{"",-1},{"99999999999999999999",-1}}){r.m_Args={s};CHECK(r.GetInt(0)==v);CHECK(r.NumArguments()==1);CHECK(r.GetString(0)==s);}
}
TEST_CASE("integration.console.files") {
 Sandbox s;CConsole c;std::vector<std::string> seen;c.Register("x",{},0,[&](auto& r){seen.push_back(r.GetString(0));},"");
 write_file("commands","# comment\nx first\nx \"second value\"\nx last");std::string p="commands";c.ExecuteFile(p);
 CHECK(seen==std::vector<std::string>{"first","second value","last"});p="absent";CHECK_NOTHROW(c.ExecuteFile(p));
}
TEST_CASE("unit.console.interactions") {
 CConsole c;std::vector<std::string> args;int flags=0;const dpp::interaction_create_t* event=nullptr;
 c.Register("x",{},BUTTON|MODAL|SLASH_COMMAND,[&](auto& r){args=r.m_Args;flags=r.m_Flags;event=r.m_Event;},"");
 dpp::button_click_t b;b.custom_id="x 42 yes";c.ExecuteInteraction(b);CHECK(flags==BUTTON);CHECK(event==&b);CHECK(args==std::vector<std::string>{"42","yes"});
 dpp::form_submit_t f;f.custom_id="x";dpp::component field;field.value=std::string("quote \" \\ ; # spaces");f.components={field};c.ExecuteInteraction(f);CHECK(flags==MODAL);CHECK(args==std::vector<std::string>{"quote \" \\ ; # spaces"});
 dpp::slashcommand_t sl;sl.command.name="x";c.ExecuteInteraction(sl);CHECK(flags==SLASH_COMMAND);CHECK(args.empty());
 dpp::interaction_create_t other;c.ExecuteInteraction(other);CHECK(flags==SLASH_COMMAND);
}
TEST_CASE("integration.config.defaults_and_roundtrip") {
 Sandbox s;
 {CConsole c;CConfig cfg;CHECK(cfg.ROLE_ID==1000);CHECK(cfg.ClanTag=="Peace Duke");cfg.OnInit(&c);line(c,"role_id 18446744073709551615; clan_tag \"Test Clan\"; about \"Hello world\"");CHECK(cfg.ROLE_ID==UINT64_MAX);CHECK(cfg.About=="Hello world");line(c,"role_id bad");CHECK(cfg.ROLE_ID==UINT64_MAX);line(c,"role_id");}
 {CConsole c;CConfig cfg;cfg.OnInit(&c);CHECK(cfg.ROLE_ID==UINT64_MAX);CHECK(cfg.ClanTag=="Test Clan");CHECK(cfg.About=="Hello world");}
}
TEST_CASE("unit.serialization.user_and_optional") {
 auto u=sample();nlohmann::json j=u;CHECK(j.get<SUserData>().m_GameNick==u.m_GameNick);CHECK(j.get<SUserData>().m_BirthDay.m_Year==2000);CHECK(j.get<SUserData>().m_SocialRating==-7);
 u.m_Age.reset();u.m_BirthDay.m_Year.reset();j=u;CHECK(j["m_Age"].is_null());CHECK_FALSE(j.get<SUserData>().m_Age);CHECK_FALSE(j.get<SUserData>().m_BirthDay.m_Year);
 CHECK_THROWS_AS(nlohmann::json::object().get<SUserData>(),nlohmann::json::exception);
 j["m_Id"]="invalid";CHECK_THROWS_AS(j.get<SUserData>(),nlohmann::json::exception);
}
TEST_CASE("unit.logger.levels") {
 Capture c;CLogger::Info("test","info");CLogger::Warning("test","warn");CLogger::Error("test","error");CLogger::Debug("test","debug");
 CHECK(c.out.str().find("[inf] test: info")!=std::string::npos);CHECK(c.out.str().find("[wrn] test: warn")!=std::string::npos);CHECK(c.err.str().find("[err] test: error")!=std::string::npos);
#ifndef NDEBUG
 CHECK(c.out.str().find("[dbg] test: debug")!=std::string::npos);
#endif
}
TEST_CASE("integration.database.crud_and_persistence") {
 Sandbox s;auto u=sample();
 {CJsonDataBase db;db.Connect("db");CHECK_FALSE(db.Load<SUserData>("users",42));CHECK(db.GetKeys("missing").empty());db.Save("users",42,u);REQUIRE(db.Load<SUserData>("users",42));CHECK(db.Load<SUserData>("users",42)->m_About==u.m_About);u.m_GameNick="updated";db.Save("users",42,u);CHECK(db.GetKeys("users")==std::vector<size_t>{42});db.Save("users",99,u);db.Erase("users",99);db.Erase("missing",1);db.Erase("users",123);CHECK_FALSE(db.Load<SUserData>("users",99));std::this_thread::sleep_for(std::chrono::milliseconds(650));}
 {CJsonDataBase db;db.Connect("db.json");REQUIRE(db.Load<SUserData>("users",42));CHECK(db.Load<SUserData>("users",42)->m_GameNick=="updated");}
}
TEST_CASE("integration.database.invalid_json_and_types") {
 Sandbox s;write_file("bad.json","{not json");
 {CJsonDataBase db;db.Connect("bad.json");CHECK(db.GetKeys("x").empty());db.Save("x",1,42);CHECK(db.Load<int>("x",1)==42);CHECK_FALSE(db.Load<SUserData>("x",1));db.Save("x",2,nullptr);CHECK_FALSE(db.Load<int>("x",2));db.Save("x",3,nlohmann::json::object());CHECK_FALSE(db.Load<int>("x",3));}
}
TEST_CASE("integration.database.key_generation") {
 Sandbox s;{CJsonDataBase db;db.Connect("db");CHECK(db.GenerateNewKey("a")==1);CHECK(db.GenerateNewKey("a")==2);CHECK(db.GenerateNewKey("b")==1);CHECK(db.GetKeys("a").empty());std::this_thread::sleep_for(std::chrono::milliseconds(650));}
 {CJsonDataBase db;db.Connect("db");CHECK(db.GenerateNewKey("a")==3);}
}
TEST_CASE("integration.database.readonly_save_erase") {
 Sandbox s;write_file("db.json",R"({"a":{"1":42}})");CJsonDataBase db;db.Connect("db",ReadOnly);CHECK(db.Load<int>("a",1)==42);CHECK_THROWS_AS(db.Save("a",2,43),std::logic_error);CHECK_THROWS_AS(db.Erase("a",1),std::logic_error);
}
TEST_CASE("integration.database.malformed_keys") {
 Sandbox s;write_file("db.json",R"({"a":{"1":1,"bad":2,"999999999999999999999999":3,"_meta":{"next_id":7}},"scalar":4})");CJsonDataBase db;db.Connect("db");CHECK(db.GetKeys("a")==std::vector<size_t>{1});CHECK(db.GetKeys("scalar").empty());CHECK(db.GenerateNewKey("a")==7);
}
TEST_CASE("integration.database.unwritable_path") {Sandbox s;CJsonDataBase db;Capture c;db.Connect("missing/db");db.Save("x",1,1);CHECK(db.Load<int>("x",1)==1);CHECK(c.err.str().find("Failed to sync")!=std::string::npos);}
TEST_CASE("stress.database.parallel_writers") {
 Sandbox s;CJsonDataBase db;db.Connect("db");std::vector<std::thread> threads;std::atomic<int> errors=0;
 for(int t=0;t<8;++t)threads.emplace_back([&]{for(int i=0;i<100;++i){auto k=db.GenerateNewKey("a");db.Save("a",k,k);if(db.Load<size_t>("a",k)!=k)++errors;}});
 for(auto& t:threads)t.join();CHECK(errors==0);CHECK(db.GetKeys("a").size()==800);CHECK(db.GenerateNewKey("a")==801);
}
