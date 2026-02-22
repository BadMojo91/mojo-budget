cmake_minimum_required(VERSION 3.10)
project(mojo-budget C)

set(CMAKE_C_STANDARD 11)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# CImGui (installed globally)
include_directories(/usr/include)

# STB (header-only, installed globally, usually /usr/include)
include_directories(/usr/include)

file(GLOB SRC_FILES src/*.c)

add_executable(mojo-budget ${SRC_FILES})
target_link_libraries(mojo-budget ${SDL2_LIBRARIES} ncurses)
