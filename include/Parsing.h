#include <iostream>
#include <string>

class Parsing
{
private:
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
	static std::string fetch_html(const std::string& url);
	static std::string UrlCode(const std::string &value);
public:
	static std::string GetUrl(std::string name);
	static int GetPoints(std::string url);
};