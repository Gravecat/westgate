# 3rdparty/fantasyname/CMakeLists.txt -- Very basic minimal CMake file for building fantasyname code as its own object.

# This file is dedicated to the public domain under CC0 1.0 Universal. To the extent possible under law,
# Raine "Gravecat" Simmons has waived all copyright and related or neighbouring rights to this file.
# See https://creativecommons.org/publicdomain/zero/1.0/ for details.

cmake_minimum_required(VERSION 3.10)

add_library(fantasyname STATIC namegen.cpp)
target_compile_definitions(fantasyname PRIVATE
  HAVE_CXX14=1
  _CRT_SECURE_NO_WARNINGS
  $<$<BOOL:${TARGET_WINDOWS}>:WESTGATE_TARGET_WINDOWS>
  $<$<BOOL:${TARGET_MINGW}>:WESTGATE_TARGET_MINGW>
  $<$<BOOL:${TARGET_LINUX}>:WESTGATE_TARGET_LINUX>
  $<$<BOOL:${TARGET_APPLE}>:WESTGATE_TARGET_APPLE>
)
