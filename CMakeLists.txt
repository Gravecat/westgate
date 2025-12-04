# CMakeLists.txt -- CMake build file for Westgate. Mainly built for GCC but should support
# some other popular compilers too.

# SPDX-FileType: SOURCE
# SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(westgate LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files.
set(WESTGATE_CPPS
  src/actions/cheats.cpp
  src/actions/meta.cpp
  src/actions/silly.cpp
  src/actions/world-interaction.cpp
  src/core/core.cpp
  src/core/game.cpp
  src/core/terminal.cpp
  src/parser/parser.cpp
  src/util/binpath.cpp
  src/util/filex.cpp
  src/util/namegen.cpp
  src/util/strx.cpp
  src/util/timer.cpp
  src/util/yaml.cpp
  src/world/area/automap.cpp
  src/world/area/link.cpp
  src/world/area/region.cpp
  src/world/area/room.cpp
  src/world/entity/entity.cpp
  src/world/entity/mobile.cpp
  src/world/entity/player.cpp
  src/world/time/time-weather.cpp
  src/world/world.cpp
)

# Third-party code being built as separate object files.
add_subdirectory(src/3rdparty/fantasyname)
add_subdirectory(src/3rdparty/murmurhash3)
add_subdirectory(src/3rdparty/rapidyaml)

# Tell CMake to move the finished binary into a 'bin' folder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Automatically generate version strings used in-game and in the Windows resource file, from this specified game version.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/version.txt" WESTGATE_VERSION)
string(REPLACE "." "," WESTGATE_VERSION_COMMA "${WESTGATE_VERSION}")
execute_process(COMMAND date "+%y%m%d%H%M" OUTPUT_VARIABLE BUILD_TIMESTAMP OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/resource.rc.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/resource.rc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/source.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/source.hpp @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/version.hpp @ONLY)

# An extra marker for the source files, to tell what kind of build type we're using.
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type was not specified, switching to 'Debug'.")
  set(CMAKE_BUILD_TYPE Debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  message(STATUS "Build type specified as release build.")
  set(RELEASE_BUILD 1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  message(STATUS "Build type specified as debug build.")
  set(DEBUG_BUILD 1)
else()
  message(FATAL_ERROR "Unrecognized build type!")
endif()

# Determine target platform.
if(WIN32)
  set(TARGET_WINDOWS 1)
  message(STATUS "Windows build environment detected.")
elseif(MINGW)
  set(TARGET_WINDOWS 1)
  set(TARGET_MINGW 1)
  message(STATUS "MinGW build environment detected.")
elseif(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(TARGET_APPLE 1)
  message(STATUS "Apple build environment detected.")
elseif(UNIX)
  set(TARGET_LINUX 1)
  message(STATUS "Unix/Linux build environment detected.")
else()
  message(WARNING "Could not determine build environment.")
endif()

# Detect if MinGW is being used to cross-compile to Windows. There are simpler ways, but this way guarantees success when using WSL.
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
  #ifdef __MINGW32__
    int main() { return 0; }
  #else
    #error Not MinGW
  #endif
" HAVE_MINGW_MACRO)
if (HAVE_MINGW_MACRO)
  set(TARGET_WINDOWS 1)
  set(TARGET_MINGW 1)
  unset(TARGET_LINUX)
  message(STATUS "Extra checks have determined MinGW is being used. Changing target to MinGW/Windows.")
endif()

# Compiler definitions, including target platform and build type.
add_compile_definitions(
  $<$<BOOL:${TARGET_WINDOWS}>:WESTGATE_TARGET_WINDOWS>
  $<$<BOOL:${TARGET_MINGW}>:WESTGATE_TARGET_MINGW>
  $<$<BOOL:${TARGET_LINUX}>:WESTGATE_TARGET_LINUX>
  $<$<BOOL:${TARGET_APPLE}>:WESTGATE_TARGET_APPLE>
  $<$<BOOL:${RELEASE_BUILD}>:WESTGATE_BUILD_RELEASE>
  $<$<BOOL:${DEBUG_BUILD}>:WESTGATE_BUILD_DEBUG>
  __STDC_LIMIT_MACROS
  _USE_MATH_DEFINES
)

# RC compiler, for building the application icon.
if(TARGET_WINDOWS)
  if(TARGET_MINGW)  # MinGW cross-compilation
    find_program(CMAKE_RC_COMPILER
      NAMES x86_64-w64-mingw32-windres i686-w64-mingw32-windres windres
      DOC "Resource compiler for MinGW targets"
    )
    if(NOT CMAKE_RC_COMPILER)
      message(FATAL_ERROR "windres not found - cannot compile resources")
    endif()
  else()    # MSVC or Clang
    # CMake already knows where rc.exe lives; just enable the language
    set(CMAKE_RC_COMPILER_INIT rc)
  endif(TARGET_MINGW)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  set(WESTGATE_RC "${CMAKE_CURRENT_BINARY_DIR}/cmake/resource.rc")
endif(TARGET_WINDOWS)

# Binary file output. WESTGATE_RC should be blank for non-Windows builds.
add_executable(westgate ${WESTGATE_CPPS} ${WESTGATE_RC})
target_link_libraries(westgate PRIVATE
  ${CMAKE_THREAD_LIBS_INIT}
  fantasyname
  murmurhash3
  rapidyaml
)

# Include directories.
target_include_directories(westgate PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_BINARY_DIR}"
)

# Precompiled global header.
target_precompile_headers(westgate PRIVATE src/core/pch.hpp)

# Compiler-specific flags.
option(WARNINGS_AS_ERRORS "Turn warnings into errors" OFF)
target_compile_options(westgate PRIVATE
  # GNU / Clang / Apple Clang
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
    -Wall -Wextra -pedantic -pedantic-errors
    $<$<BOOL:${WARNINGS_AS_ERRORS}>:-Werror>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-g -Og>
  >
  # MSVC
  $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /permissive- /wd4244 /wd4267
    $<$<BOOL:${WARNINGS_AS_ERRORS}>:/WX>
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od /Zi>
  >
)

# Compiler-specific stripping of symbols.
target_link_options(westgate PRIVATE
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-s>
)

# Windows-specific stuff.
if(TARGET_WINDOWS)
  add_compile_definitions(
    NOMINMAX
    VC_EXTRALEAN
    WIN32_LEAN_AND_MEAN
  )
  target_link_libraries(westgate PRIVATE
    $<$<BOOL:${TARGET_MINGW}>:mingw32>
	shlwapi
	ntdll
    murmurhash3
    rapidyaml
  )
endif(TARGET_WINDOWS)
