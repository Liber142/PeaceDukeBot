#pragma once
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
namespace cpr {
struct Url{std::string str;};using Parameters=std::vector<std::pair<std::string,std::string>>;
struct Response{long status_code=200;std::string text="{}";};
inline std::function<Response(const Url&,const Parameters&)> handler;inline int requests=0;
inline Response Get(const Url& u,const Parameters& p){++requests;if(handler)return handler(u,p);return {};}
struct CurlHolder {std::string urlEncode(const std::string& s){std::ostringstream o;for(unsigned char c:s){if(std::isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~')o<<c;else o<<'%'<<std::uppercase<<std::hex<<std::setw(2)<<std::setfill('0')<<int(c);}return o.str();}};
}
