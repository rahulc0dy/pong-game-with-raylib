cmake_minimum_required(VERSION 3.31)
project(pong_game_raylib)

set(CMAKE_CXX_STANDARD 20)

include(fetchcontent)
fetchcontent_declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

add_executable(pong_game_raylib
        src/main.cpp
        src/ball.cpp
        include/ball.h
        src/paddle.cpp
        include/paddle.h
)

fetchcontent_makeavailable(raylib)

# Include headers
target_include_directories(pong_game_raylib PRIVATE
        include
)

# Link to GLFW statically
target_link_libraries(pong_game_raylib PRIVATE
        raylib
)

# Link to OpenGL on Windows
if (WIN32)
    target_link_libraries(pong_game_raylib PRIVATE opengl32)
endif ()
