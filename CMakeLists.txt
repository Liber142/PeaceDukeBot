cmake_minimum_required(VERSION 3.20)  # Минимум 3.20 для лучшей совместимости

project(DiscrodBot VERSION 0.1.0 LANGUAGES CXX)

# Явно указываем необходимые версии
find_package(dpp REQUIRED)
find_package(fmt REQUIRED)
# find_package(nlohmann_json REQUIRED)

find_library(CURL_LIB NAMES curl)
find_path(CURL_INCLUDE_DIR NAMES curl/curl.h)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/Command)
include_directories(${CMAKE_SOURCE_DIR}/src/Events)
include_directories(${CMAKE_SOURCE_DIR}/src/Votes)


file(GLOB SOURCES
       "src/Command/*.cpp"
       "src/Events/*.cpp"
       "src/Votes/*.cpp"
       "src/*.cpp"
       "main.cpp"
   )

add_executable(PeaceDukeBote ${SOURCES})

target_include_directories(PeaceDukeBote PRIVATE include)

target_link_libraries(PeaceDukeBote PRIVATE
    dpp::dpp
    fmt::fmt
    ${CMAKE_THREAD_LIBS_INIT}
    )

target_link_libraries(PeaceDukeBote PRIVATE ${CURL_LIB})
target_include_directories(PeaceDukeBote PRIVATE ${CURL_INCLUDE_DIR})
