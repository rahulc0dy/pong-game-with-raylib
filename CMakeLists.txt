cmake_minimum_required(VERSION 3.31)
project(FlappyBird)

set(CMAKE_CXX_STANDARD 20)

add_executable(FlappyBird
        src/main.cpp
        src/ball.cpp
        src/ball.h
)

set_target_properties(FlappyBird PROPERTIES LINK_FLAGS "-mconsole")

# Include headers
target_include_directories(FlappyBird PRIVATE
        include
)

# Link to GLFW statically
target_link_libraries(FlappyBird PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libraylib.a
        winmm
)

# Link to OpenGL on Windows
if (WIN32)
    target_link_libraries(FlappyBird PRIVATE opengl32)
endif()
