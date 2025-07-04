cmake_minimum_required(VERSION 3.20)  # Минимум 3.20 для лучшей совместимости

project(DiscrodBot VERSION 0.1.0 LANGUAGES CXX)

# Явно указываем необходимые версии
include_directories(/home/mrwiskis/libraries/nlohmann-json-3.11.2/include)
find_package(dpp 10.1.2 REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)

find_library(CURL_LIB NAMES curl)
find_path(CURL_INCLUDE_DIR NAMES curl/curl.h)




set(SOURCES
    src/Command/EventCommand.cpp
    src/Command/ProfileCommand.cpp
    src/Command/Apply.cpp
    src/Command/invite.cpp
    src/Events/RegisterEvent.cpp
    src/Parsing.cpp
    src/BotCore.cpp
    src/ClanApplication.cpp
    src/CommandHandler.cpp 
    src/DataBase.cpp 
    src/ModsVote.cpp
    src/Online.cpp
    main.cpp
)

add_executable(PeaceDukeBote ${SOURCES})

target_include_directories(PeaceDukeBote PRIVATE include)

target_link_libraries(PeaceDukeBote PRIVATE
    dpp::dpp
    fmt::fmt
    ${CMAKE_THREAD_LIBS_INIT}
    nlohmann_json::nlohmann_json)

target_link_libraries(PeaceDukeBote PRIVATE ${CURL_LIB})
target_include_directories(PeaceDukeBote PRIVATE ${CURL_INCLUDE_DIR})