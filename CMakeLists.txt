# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.22)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(ENGINE_SRC
    src/engine/config_variables.h
    src/engine/config.h
    src/engine/config.cpp
    src/engine/database.h
    src/engine/json_database.h
    src/engine/json_database.cpp
)

set(COMMANDS_SRC
    src/modules/commands/command.h
    src/modules/commands/apply.h
    src/modules/commands/apply.cpp
)

set(MODULES_SRC
    src/modules/module.h
    ${COMMANDS_SRC}
    src/modules/command_handler.h
    src/modules/command_handler.cpp
)

set(BOT_SRC
    src/bot_core.h
    src/bot_core.cpp
    src/main.cpp
)

set(SRC 
    ${ENGINE_SRC}
    ${BOT_SRC}
    ${MODULES_SRC}
)

# Create an executable
add_executable(${PROJECT_NAME}
    ${SRC}
)

# Find our pre-installed DPP package (using FindDPP.cmake).
find_package(DPP REQUIRED)

# Link the pre-installed DPP package.
target_link_libraries(${PROJECT_NAME} 
	${DPP_LIBRARIES}
)

# Include the DPP directories.
target_include_directories(${PROJECT_NAME} PRIVATE
	${DPP_INCLUDE_DIR}
)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

