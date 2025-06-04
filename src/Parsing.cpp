#include <curl/curl.h>
#include <string>
#include <sstream>
#include <cstddef>

#include "../include/Parsing.h"

size_t Parsing::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) 
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Parsing::fetch_html(const std::string& url) 
{
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Разрешаем редиректы
        
        // Устанавливаем User-Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

std::string Parsing::UrlCode(const std::string &value)
{
	
	std::string encoded;
	for (char c : value) 
	{
	    std::stringstream ss;
	    ss << "-" << static_cast<int>(static_cast<unsigned char>(c)) << "-";
	    encoded += ss.str();
	}
	return encoded;
}

std::string Parsing::GetUrl(std::string name)
{
	std::string url = "https://ddnet.org/players/" + UrlCode(name) + "/";
	return url;
}



int Parsing::GetPoints(std::string url)
{
	std::string html = fetch_html(url);
	if (!html.empty())
	{
		if (html.find("<title>Player not found") != std::string::npos)
		{
			std::cout << "Player not found" << std::endl;
			return -1;
		}
		else
		{
			int startPos = html.find("with", html.find("<h3>Points (")) + 5;
			int length = html.find(" points", startPos) - startPos;
			int points = std::stoi(html.substr(startPos, length)) ;
			return points;
		}
	}
	return 0;
}