cmake_minimum_required(VERSION 3.10)
project(mojo-budget LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(FETCHCONTENT_BASE_DIR ${DEPS_DIR})
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# Fetch cimgui (includes imgui as submodule)
FetchContent_Declare(
  cimgui
  GIT_REPOSITORY https://github.com/cimgui/cimgui.git
  GIT_TAG master
)
FetchContent_MakeAvailable(cimgui)
set(CIMGUI_BACKENDS
    ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_opengl2.cpp
)


file(GLOB SRC_NCURSES src/main_ncurses.c)
file(GLOB SRC_SDL src/main_sdl.c src/sdl/*.c)
file(GLOB SRC_FILES src/budget/*.c)

message(STATUS "SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "cimgui source dir: ${cimgui_SOURCE_DIR}")
message(STATUS "imgui backends: ${CIMGUI_BACKENDS}")

add_executable(mojo-budget-ncurses ${SRC_NCURSES} ${SRC_FILES})
target_link_libraries(mojo-budget-ncurses ncurses panel)

add_executable(mojo-budget-sdl ${SRC_SDL} ${SRC_FILES} ${CIMGUI_BACKENDS})
target_link_libraries(mojo-budget-sdl ${SDL2_LIBRARIES} cimgui OpenGL::GL)
target_include_directories(mojo-budget-sdl PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${cimgui_SOURCE_DIR}
    ${cimgui_SOURCE_DIR}/imgui/backends
)
