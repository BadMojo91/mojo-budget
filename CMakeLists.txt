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
# Use build-dir-relative path for FetchContent builds so Linux and Windows
# builds don't overwrite each other's compiled artifacts.
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
# If sources were previously cached in the source-tree deps/ dir, reuse them.
foreach(_dep libxlsx zlib stb cimgui tinyfiledialogs)
  if(EXISTS "${DEPS_DIR}/${_dep}-src")
    string(TOUPPER "${_dep}" _dep_upper)
    set(FETCHCONTENT_SOURCE_DIR_${_dep_upper} "${DEPS_DIR}/${_dep}-src")
  endif()
endforeach()

# On Windows/MinGW, point SDL2 at the bundled MinGW dev package
if(WIN32)
  set(SDL2_MINGW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/sdl2-mingw/x86_64-w64-mingw32"
      CACHE PATH "SDL2 MinGW root directory")
  set(SDL2_DIR "${SDL2_MINGW_ROOT}/lib/cmake/SDL2" CACHE PATH "SDL2 cmake dir" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
set(IMGUI_STATIC TRUE CACHE BOOL "Build ImGui as static library" FORCE)

# On Windows/MinGW, zlib must be built from source since it's not provided by the toolchain.
if(WIN32)
  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
  )
  FetchContent_MakeAvailable(zlib)

  # Create ZLIB::ZLIB alias so libxlsxwriter's find_package(ZLIB) finds it.
  # FindZLIB.cmake skips creating the imported target if ZLIB::ZLIB already exists.
  if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
  endif()
  set(ZLIB_FOUND        TRUE                                              CACHE BOOL     "" FORCE)
  set(ZLIB_LIBRARY      "${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a" CACHE FILEPATH "" FORCE)
  set(ZLIB_INCLUDE_DIR  "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}"           CACHE STRING   "" FORCE)
  set(ZLIB_VERSION_STRING "1.3.1"                                         CACHE STRING   "" FORCE)
endif()

FetchContent_Declare(
  libxlsx
  GIT_REPOSITORY https://github.com/jmcnamara/libxlsxwriter.git
  GIT_TAG v1.2.4
)
FetchContent_MakeAvailable(libxlsx)

# stb headers - used for stb_ds (dynamic arrays). System-provided on Linux;
# fetched here for cross-platform builds (e.g., MinGW).
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# stb files land flat in stb_SOURCE_DIR, but source includes them as <stb/stb_ds.h>.
# Create a stb/ subdir in the build tree so the include path resolves correctly.
file(COPY ${stb_SOURCE_DIR}/stb_ds.h DESTINATION ${CMAKE_BINARY_DIR}/stb-include/stb/)
set(STB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/stb-include)

file(GLOB SRC_NCURSES CONFIGURE_DEPENDS src/ncurses/*.c)
file(GLOB SRC_SDL CONFIGURE_DEPENDS src/sdl/*.c)
file(GLOB SRC_FILES CONFIGURE_DEPENDS src/core/*.c)
if(BUILD_NCURSES)
  find_package(Curses REQUIRED)

  add_executable(mojo-budget-ncurses ${SRC_NCURSES} ${SRC_FILES})
  target_include_directories(mojo-budget-ncurses PRIVATE ${STB_INCLUDE_DIR})
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
      ${STB_INCLUDE_DIR}
  )

  # SDL2 MinGW cmake config sets include path to .../include/SDL2, but source
  # uses <SDL2/SDL.h>, so we need to also add the parent include directory.
  if(WIN32)
    target_include_directories(mojo-budget-sdl PRIVATE "${SDL2_MINGW_ROOT}/include")
  endif()

  if(TARGET SDL2::SDL2)
    # SDL2main must be linked BEFORE SDL2 to resolve entry point references
    if(TARGET SDL2::SDL2main)
      target_link_libraries(mojo-budget-sdl PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(mojo-budget-sdl PRIVATE SDL2::SDL2)
  else()
    target_include_directories(mojo-budget-sdl PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(mojo-budget-sdl PRIVATE ${SDL2_LIBRARIES})
  endif()

  target_link_libraries(mojo-budget-sdl PRIVATE cimgui OpenGL::GL xlsxwriter)

  if(WIN32)
    # Statically link MinGW runtimes so no libgcc/libstdc++/libwinpthread DLLs are needed
    target_link_options(mojo-budget-sdl PRIVATE
      -static-libgcc
      -static-libstdc++
      -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic
    )
    # Link required Windows system libraries and static zlib (built from source).
    # Use --start-group/--end-group to resolve circular references between archives.
    target_link_libraries(mojo-budget-sdl PRIVATE
      -Wl,--start-group
      zlibstatic
      mingw32
      -Wl,--end-group
      -mwindows      # Use Windows subsystem (no console window)
      opengl32
      imm32
      winmm
      version
      setupapi
    )
    # Copy SDL2.dll next to the exe after build
    add_custom_command(TARGET mojo-budget-sdl POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_MINGW_ROOT}/bin/SDL2.dll"
        "$<TARGET_FILE_DIR:mojo-budget-sdl>/SDL2.dll"
      COMMENT "Copying SDL2.dll"
    )
  endif()
endif()

message(STATUS "Configured frontends: ncurses=${BUILD_NCURSES}, sdl=${BUILD_SDL}")
