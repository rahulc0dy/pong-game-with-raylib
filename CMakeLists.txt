cmake_minimum_required(VERSION 3.31)
project(pong_game_raylib)

set(CMAKE_CXX_STANDARD 20)

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
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

fetchcontent_makeavailable(raylib enet raygui)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

# Include headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        include
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