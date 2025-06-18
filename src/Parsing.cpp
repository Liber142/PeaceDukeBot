#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>
#include <sstream>
#include <cstddef>

#include "../include/Parsing.h"

using json = nlohmann::json;

size_t Parsing::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) 
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Parsing::fetchData(const std::string& url) 
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) 
	{
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::string Parsing::UrlCode(const std::string &value) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex; // Устанавливаем шестнадцатеричный формат

    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') 
	{
            encoded << c; // Оставляем символ как есть, если он допустим в URL
        }
       	else
       	{
            encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}

std::string Parsing::GetUrl(std::string name)
{
	std::string url = "https://ddnet.org/players/?json2=" + UrlCode(name) ;
	std::cout << "url: " << url << std::endl;
	return url;
}


int Parsing::GetPoints(std::string url)
{
	std::cout << "Вход в GetPoints(std::string url)" << std::endl;
    std::string strData = fetchData(url);
    //std::cout << "Response data: " << strData << std::endl; // Debugging output

    json jsonData = json::parse(strData);

    if (jsonData.contains("points") && jsonData["points"].contains("points")) 
    {
        auto points = jsonData["points"]["points"];
	std::cout << "GetPoints(std::string url) point: " << points << std::endl;
        return points.get<int>();
    }
    else 
    {
        std::cout << "The JSON data does not contain the 'points' key." << std::endl;
        return 0; // or handle the error appropriately
    }
}
