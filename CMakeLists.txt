cmake_minimum_required(VERSION 3.16)
project(mojo-budget LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

option(BUILD_NCURSES "Build ncurses frontend" ON)
option(BUILD_SDL "Build SDL frontend" ON)

if(NOT BUILD_NCURSES AND NOT BUILD_SDL)
  message(FATAL_ERROR "At least one frontend must be enabled: BUILD_NCURSES or BUILD_SDL")
endif()

set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(FETCHCONTENT_BASE_DIR ${DEPS_DIR})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
set(IMGUI_STATIC TRUE CACHE BOOL "Build ImGui as static library" FORCE)

FetchContent_Declare(
  libxlsx
  GIT_REPOSITORY https://github.com/jmcnamara/libxlsxwriter.git
  GIT_TAG v1.2.4
)
FetchContent_MakeAvailable(libxlsx)

file(GLOB SRC_NCURSES CONFIGURE_DEPENDS src/ncurses/*.c)
file(GLOB SRC_SDL CONFIGURE_DEPENDS src/sdl/*.c)
file(GLOB SRC_FILES CONFIGURE_DEPENDS src/core/*.c)
if(BUILD_NCURSES)
  find_package(Curses REQUIRED)

  add_executable(mojo-budget-ncurses ${SRC_NCURSES} ${SRC_FILES})
  if(TARGET Curses::Curses)
    target_link_libraries(mojo-budget-ncurses PRIVATE Curses::Curses)
  else()
    target_include_directories(mojo-budget-ncurses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(mojo-budget-ncurses PRIVATE ${CURSES_LIBRARIES})
  endif()

  find_library(PANEL_LIBRARY panel)
  if(PANEL_LIBRARY)
    target_link_libraries(mojo-budget-ncurses PRIVATE ${PANEL_LIBRARY})
  endif()

  target_link_libraries(mojo-budget-ncurses PRIVATE xlsxwriter)
endif()

if(BUILD_SDL)
  find_package(SDL2 REQUIRED)
  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL REQUIRED)

  FetchContent_Declare(
    cimgui
    GIT_REPOSITORY https://github.com/cimgui/cimgui.git
    GIT_TAG b6563e1b115a08e4e6651576420600df225e0cae
  )
  FetchContent_MakeAvailable(cimgui)

  set(CIMGUI_BACKENDS
      ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
      ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_opengl2.cpp
  )

  FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
    GIT_TAG fe637654492cb8257cb68a8025dfc09ce3e0f490
  )
  FetchContent_MakeAvailable(tinyfiledialogs)

  add_executable(
    mojo-budget-sdl
    ${SRC_SDL}
    ${SRC_FILES}
    ${CIMGUI_BACKENDS}
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
  )

  target_include_directories(
    mojo-budget-sdl
    PRIVATE
      ${cimgui_SOURCE_DIR}
      ${cimgui_SOURCE_DIR}/imgui
      ${cimgui_SOURCE_DIR}/imgui/backends
      ${tinyfiledialogs_SOURCE_DIR}
      ${xlsxwriter_SOURCE_DIR}/include
  )

  if(TARGET SDL2::SDL2)
    target_link_libraries(mojo-budget-sdl PRIVATE SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
      target_link_libraries(mojo-budget-sdl PRIVATE SDL2::SDL2main)
    endif()
  else()
    target_include_directories(mojo-budget-sdl PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(mojo-budget-sdl PRIVATE ${SDL2_LIBRARIES})
  endif()

  target_link_libraries(mojo-budget-sdl PRIVATE cimgui OpenGL::GL xlsxwriter)
endif()

message(STATUS "Configured frontends: ncurses=${BUILD_NCURSES}, sdl=${BUILD_SDL}")
