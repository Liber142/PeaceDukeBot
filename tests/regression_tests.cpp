#include "support.h"
#include <bot_core.h>
#include <cpr/cpr.h>
// These assert intended contracts and deliberately remain failing for existing defects.
TEST_CASE("regression.console.empty_quoted_argument") {CConsole c;std::vector<std::string> args;c.Register("x",{},0,[&](auto& r){args=r.m_Args;},"");line(c,"x \"\" tail");CHECK(args==std::vector<std::string>{"","tail"});}
TEST_CASE("regression.console.trailing_escape") {CConsole c;c.Register("x",{},0,[](auto&){},"");CHECK_NOTHROW(line(c,"x trailing\\"));}
TEST_CASE("regression.console.dispatch_respects_flags") {CConsole c;int button=0,slash=0;c.Register("x",{},BUTTON,[&](auto&){++button;},"");c.Register("x",{},SLASH_COMMAND,[&](auto&){++slash;},"");dpp::slashcommand_t e;e.command.name="x";c.ExecuteInteraction(e);CHECK(slash==1);CHECK(button==0);}
TEST_CASE("regression.console.zero_flag_duplicate") {CConsole c;c.Register("x",{},0,[](auto&){},"");c.Register("x",{},0,[](auto&){},"");CHECK(c.GetAllCommands().size()==1);}
TEST_CASE("regression.config.escaped_string_roundtrip") {Sandbox s;std::string expected="quote \" slash \\ newline\nend";{CConfig c;c.About=expected;}{CConsole console;CConfig c;c.OnInit(&console);CHECK(c.About==expected);}}
TEST_CASE("regression.config.negative_id_rejected") {Sandbox s;CConfig cfg;CConsole c;cfg.OnInit(&c);auto before=cfg.ROLE_ID;line(c,"role_id -1");CHECK(cfg.ROLE_ID==before);}
TEST_CASE("regression.database.unconnected_destructor") {CHECK_NOTHROW([]{CJsonDataBase db;}());}
TEST_CASE("regression.database.readonly_does_not_rewrite") {Sandbox s;std::string original="{\n  \"a\": {\"1\": 42}\n}\n";write_file("db.json",original);{CJsonDataBase db;db.Connect("db",ReadOnly);std::this_thread::sleep_for(std::chrono::milliseconds(650));}CHECK(read_file("db.json")==original);}
TEST_CASE("regression.database.readonly_missing_does_not_create") {Sandbox s;{CJsonDataBase db;db.Connect("missing",ReadOnly);}CHECK_FALSE(fs::exists("missing.json"));}
TEST_CASE("regression.database.readonly_key_generation") {Sandbox s;write_file("db.json","{}");CJsonDataBase db;db.Connect("db",ReadOnly);CHECK_THROWS_AS(db.GenerateNewKey("a"),std::logic_error);}
TEST_CASE("regression.database.shutdown_flush") {Sandbox s;{CJsonDataBase db;db.Connect("db");db.Save("x",1,42);}CJsonDataBase db;db.Connect("db");CHECK(db.Load<int>("x",1)==42);}
TEST_CASE("regression.database.strict_numeric_keys") {Sandbox s;write_file("db.json",R"({"a":{"1x":0,"-1":0,"2":0}})");CJsonDataBase db;db.Connect("db");CHECK(db.GetKeys("a")==std::vector<size_t>{2});}
TEST_CASE("regression.database.large_key") {Sandbox s;write_file("db.json",R"({"a":{"_meta":{"next_id":2147483647}}})");CJsonDataBase db;db.Connect("db");CHECK(db.GenerateNewKey("a")==size_t(2147483647));CHECK(db.GenerateNewKey("a")==size_t(2147483648ULL));}
TEST_CASE("regression.votes.invalid_calendar_dates") {CApplyVoteManager m;for(auto s:{"31.02.2000","29.02.2001","31.04.2000","01.01.2000junk","01.01.bad","01.01.-1"}){INFO(s);CHECK_FALSE(m.ParseBirthday(s));}}
TEST_CASE("regression.profile.absent_optional_target") {CProfileCommand p;dpp::slashcommand_t e;e.command.usr.id=42;CHECK_NOTHROW(p.GetTargetUser(&e));}
TEST_CASE("regression.findtas.http_error_is_exception") {CFindTas f;cpr::handler=[](auto&,auto&){return cpr::Response{500,"error"};};CHECK_THROWS(f.Player("bad"));}
TEST_CASE("regression.music.respects_decoded_byte_count") {CMusic m;CHECK(m.DecodeMP3("synthetic.mp3").size()==3);}
TEST_CASE("stress.database.concurrent_enumeration_and_erase") {Sandbox s;CJsonDataBase db;db.Connect("db");db.Save("x",1,1);std::atomic<bool> start=false;std::thread w([&]{while(!start.load()){}for(size_t i=1;i<1000;++i){db.Save("x",i,i);db.Erase("x",i);}});start=true;for(int i=0;i<1000;++i)db.GetKeys("x");w.join();CHECK(db.GetKeys("x").empty());}
TEST_CASE("regression.members.deferred_callback_lifetime") {
 Sandbox s;dpp::cluster bot;CBotCore core(&bot);bot.defer=true;core.m_ClanMemberManager.AddClanMember(sample());
 // Real SDK callbacks are asynchronous: stack arguments must survive returning.
 std::vector<std::string> churn(1000,std::string(1000,'x'));bot.drain();
 REQUIRE(bot.edited_members.size()==1);CHECK(bot.edited_members[0].nickname==sample().m_GameNick);
}
TEST_CASE("regression.database.filename_suffix") {Sandbox s;CJsonDataBase db;db.Connect("name.json.backup");CHECK(fs::exists("name.json.backup.json"));}
TEST_CASE("regression.votes.future_birthdate") {CApplyVoteManager m;auto age=m.CalculateAge({1,1,3000});CHECK_FALSE(age.has_value());}
