#pragma once
#include <dpp/nlohmann/json.hpp>
#include <algorithm>
#include <atomic>
#include <csignal>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <unistd.h>
namespace dpp {
struct snowflake {
 uint64_t value=0; snowflake()=default; snowflake(uint64_t v):value(v){} operator uint64_t() const{return value;} bool empty()const{return !value;}
 bool operator==(const snowflake&)const=default; bool operator==(uint64_t v)const{return value==v;}
};
inline void to_json(nlohmann::json& j,const snowflake& s){j=s.value;}
inline void from_json(const nlohmann::json& j,snowflake& s){s.value=j.get<uint64_t>();}
enum component_type {cot_action_row,cot_button,cot_text,cot_text_display,cot_separator,cot_container};
enum component_style {cos_primary,cos_success,cos_danger};
enum {text_short,text_paragraph,co_user,m_ephemeral=64,m_using_components_v2=32768};
namespace colors {inline unsigned green=0x00ff00,red=0xff0000;}
struct component {
 int type=0,style=0,min_length=0,max_length=0; bool required=false,divider=false; unsigned accent=0;
 std::string label,custom_id,content,placeholder; std::variant<std::string,int> value=std::string{}; std::vector<component> components;
 component& set_type(int v){type=v;return *this;} component& set_style(int v){style=v;return *this;}
 component& set_label(std::string v){label=v;return *this;} component& set_id(std::string v){custom_id=v;return *this;}
 component& set_content(std::string v){content=v;return *this;} component& set_placeholder(std::string v){placeholder=v;return *this;}
 component& set_required(bool v){required=v;return *this;} component& set_divider(bool v){divider=v;return *this;}
 component& set_min_length(int v){min_length=v;return *this;} component& set_max_length(int v){max_length=v;return *this;}
 component& set_text_style(int){return *this;} component& set_accent(unsigned v){accent=v;return *this;}
 component& add_component(component v){components.push_back(v);return *this;} component& add_component_v2(component v){return add_component(v);}
};
struct embed {struct field {std::string name,value;bool is_inline;}; unsigned color=0;std::string title,description;std::vector<field> fields;
 embed& set_color(unsigned v){color=v;return *this;} embed& set_title(std::string v){title=v;return *this;} embed& set_description(std::string v){description=v;return *this;}
 embed& add_field(std::string n,std::string v,bool b){fields.push_back({n,v,b});return *this;}
};
struct message {snowflake id,channel_id;int flags=0;std::string content;std::vector<component> components;std::vector<embed> embeds;
 message()=default;message(std::string s):content(s){} message(snowflake c,std::string s):channel_id(c),content(s){} message(embed e):embeds{e}{}
 message& set_flags(int v){flags=v;return *this;}message& set_content(std::string v){content=v;return *this;}
 message& add_component(component c){components.push_back(c);return *this;}message& add_component_v2(component c){return add_component(c);}
};
struct interaction_modal_response {std::string custom_id,title;std::vector<component> components; interaction_modal_response(std::string i,std::string t):custom_id(i),title(t){}
 void add_component(component c){components.push_back(c);}void add_row(){}
};
struct user {snowflake id;std::string username="mock";std::string format_username()const{return username;}};
inline std::map<uint64_t,user> users;
inline const user* find_user(snowflake id){auto i=users.find(id);return i==users.end()?nullptr:&i->second;}
using command_value=std::variant<std::monostate,snowflake,std::string,int>;
struct interaction {user usr;std::string name;std::string get_command_name()const{return name;}user get_issuing_user()const{return usr;}};
struct cluster;
struct interaction_create_t {virtual ~interaction_create_t()=default; interaction command;cluster* owner=nullptr;mutable std::vector<message> replies;mutable std::vector<interaction_modal_response> dialogs;mutable bool thought=false,deleted=false;
 void reply()const{replies.emplace_back();}void reply(const message& m)const{replies.push_back(m);}void reply(std::string s)const{reply(message(s));}
 void dialog(interaction_modal_response m)const{dialogs.push_back(m);}void thinking(bool)const{thought=true;}void delete_original_response()const{deleted=true;}
};
struct button_click_t:interaction_create_t {std::string custom_id;};
struct form_submit_t:interaction_create_t {std::string custom_id;std::vector<component> components;};
struct slashcommand_t:interaction_create_t {std::map<std::string,command_value> parameters;command_value get_parameter(std::string s)const{auto i=parameters.find(s);return i==parameters.end()?command_value{}:i->second;}};
struct command_option {command_option(int,std::string,std::string,bool){}};
struct slashcommand {std::string name;slashcommand(std::string n,std::string,snowflake):name(n){}void add_option(command_option){}};
struct guild_member {snowflake guild_id,user_id;std::vector<snowflake> roles;std::string nickname;const auto& get_roles()const{return roles;}void set_nickname(std::string s){nickname=s;}};
using guild_member_map=std::map<snowflake,guild_member>;
inline bool operator<(snowflake a,snowflake b){return a.value<b.value;}
struct error_info {std::string human_readable="injected failure";};
struct confirmation_callback_t {bool error=false;std::variant<message,guild_member,guild_member_map> value;bool is_error()const{return error;}error_info get_error()const{return {};}};
using callback=std::function<void(const confirmation_callback_t&)>;
struct guild_member_add_t {guild_member added;};struct guild_member_update_t{guild_member updated;};struct ready_t{};
struct voiceclient {bool is_ready()const{return true;}void send_audio_raw(uint16_t*,size_t){}};
struct voiceconn {struct voiceclient* voiceclient=nullptr;};struct shard {voiceconn voice;voiceconn* get_voice(snowflake){return &voice;}};
struct voice_ready_t {shard* source=nullptr;shard* from()const{return source;}};
struct guild {void connect_member_voice(cluster&,snowflake){}};inline guild mock_guild;inline guild* find_guild(snowflake){return &mock_guild;}
struct cluster {
 cluster()=default;cluster(const char*,int){}
 template<class F> void on_log(F){}
 template<class F> uint64_t start_timer(F,int){return 1;}
 void stop_timer(uint64_t){}
 void start(int){if(ready)ready({});}
 void shutdown(){}
 user me;guild_member_map members;bool fail=false,defer=false;uint64_t next_message=1;
 std::vector<message> created,edited,dms;std::vector<std::tuple<snowflake,snowflake,snowflake>> added_roles,removed_roles;std::vector<guild_member> edited_members;std::vector<std::string> commands;std::vector<std::function<void()>> pending;
 std::function<void(const guild_member_add_t&)> member_add;std::function<void(const guild_member_update_t&)> member_update;
 std::function<void(const ready_t&)> ready;std::function<void(const button_click_t&)> button;std::function<void(const slashcommand_t&)> slash;std::function<void(const form_submit_t&)> form;std::function<void(const voice_ready_t&)> voice;
 void dispatch(callback cb,confirmation_callback_t r){if(!cb)return;r.error=fail;if(defer)pending.push_back([cb,r]{cb(r);});else cb(r);}
 void drain(){auto p=std::move(pending);pending.clear();for(auto& f:p)f();}
 void on_guild_member_add(decltype(member_add) f){member_add=f;}void on_guild_member_update(decltype(member_update) f){member_update=f;}
 void on_ready(decltype(ready) f){auto old=ready;ready=[old,f](const ready_t& e){if(old)old(e);f(e);};}void on_button_click(decltype(button) f){button=f;}void on_slashcommand(decltype(slash) f){slash=f;}void on_form_submit(decltype(form) f){form=f;}void on_voice_ready(decltype(voice) f){voice=f;}
 void global_command_create(slashcommand c,callback cb={}){commands.push_back(c.name);dispatch(cb,{});}void guild_command_create(slashcommand c,snowflake,callback cb={}){global_command_create(c,cb);}
 void guild_get_members(snowflake,int,int,callback cb){dispatch(cb,{false,members});}
 void guild_get_member(snowflake g,snowflake u,callback cb){auto m=members[u];m.guild_id=g;m.user_id=u;dispatch(cb,{false,m});}
 void guild_edit_member(guild_member m,callback cb){edited_members.push_back(m);dispatch(cb,{});}
 void guild_member_add_role(snowflake g,snowflake u,snowflake r,callback cb={}){added_roles.emplace_back(g,u,r);dispatch(cb,{});}
 void guild_member_delete_role(snowflake g,snowflake u,snowflake r,callback cb={}){removed_roles.emplace_back(g,u,r);dispatch(cb,{});}
 void message_create(message m,callback cb={}){m.id=next_message++;created.push_back(m);dispatch(cb,{false,m});}
 void message_edit(message m,callback cb={}){edited.push_back(m);dispatch(cb,{false,m});}
 void direct_message_create(snowflake u,message m){m.channel_id=u;dms.push_back(m);}
};
}
namespace dpp {using timer=uint64_t;enum {ll_error=3,ll_critical=4,i_default_intents=1,i_guild_members=2,st_return=0};struct log_t{int severity;std::string message;};namespace utility {inline std::string loglevel(int){return "mock";}}}
