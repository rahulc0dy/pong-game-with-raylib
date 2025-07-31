cmake_minimum_required(VERSION 3.31)
project(pong_game_raylib)

set(CMAKE_CXX_STANDARD 20)

set(GAME_PORT_NUMBER 3000 CACHE STRING "Port number for the game server")

include(fetchcontent)

# Fetch raylib
fetchcontent_declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

# Fetch raygui
fetchcontent_declare(raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG 4.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

# Fetch enet
fetchcontent_declare(enet
        GIT_REPOSITORY https://github.com/zpl-c/enet.git
        GIT_TAG v2.6.5
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

# Build  Libraries statically
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENET_STATIC ON CACHE BOOL "" FORCE)

# Get all cpp files from src directory
file(GLOB_RECURSE GAME_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp")
file(GLOB_RECURSE SERVER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp")
file(GLOB_RECURSE SHARED_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/shared/*.cpp")

fetchcontent_makeavailable(raylib enet raygui)

add_executable(${CMAKE_PROJECT_NAME}
        ${GAME_FILES}
        ${SHARED_FILES}
)

# Include headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/shared
        ${CMAKE_CURRENT_SOURCE_DIR}/include/game
        ${raygui_SOURCE_DIR}/src
)

# Link to GLFW statically
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        raylib
        enet::enet_static
)

# Link to OpenGL on Windows
if (WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE opengl32 winmm)
endif ()

# Copy assets to the build directory with custom command
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
)

add_executable(server
        ${SERVER_FILES}
        ${SHARED_FILES}
)

target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/shared
        ${CMAKE_CURRENT_SOURCE_DIR}/include/server
)

target_link_libraries(server PRIVATE
        enet::enet_static
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE PORT_NUMBER=${GAME_PORT_NUMBER})
target_compile_definitions(server PRIVATE PORT_NUMBER=${GAME_PORT_NUMBER})