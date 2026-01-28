# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.22)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(ENGINE_SRC
    src/engine/config_variables.h
    src/engine/config.h
    src/engine/config.cpp
    src/engine/console.h
    src/engine/console.cpp
    src/engine/database.h
    src/engine/json_database.h
    src/engine/json_database.cpp
    src/engine/logger.h
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

set(GEN_REVISION_FILE "${CMAKE_BINARY_DIR}/generated/src/git_revision.h")

add_custom_command(
    OUTPUT "${GEN_REVISION_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated/src"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/git_revision.py" > "${GEN_REVISION_FILE}.tmp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GEN_REVISION_FILE}.tmp" "${GEN_REVISION_FILE}"
    DEPENDS "${CMAKE_SOURCE_DIR}/scripts/git_revision.py"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Generating git revision header"
    VERBATIM
)

add_custom_target(generate_revision DEPENDS "${GEN_REVISION_FILE}")
add_dependencies(${PROJECT_NAME} generate_revision)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/generated/src")

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

