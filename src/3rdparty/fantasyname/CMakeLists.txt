# Very basic minimal CMake file for building fantasyname code as its own object.

cmake_minimum_required(VERSION 3.12)

add_library(fantasyname STATIC namegen.cpp)
target_compile_definitions(fantasyname PRIVATE
  HAVE_CXX14=1
  _CRT_SECURE_NO_WARNINGS
  $<$<BOOL:${TARGET_WINDOWS}>:WESTGATE_TARGET_WINDOWS>
  $<$<BOOL:${TARGET_MINGW}>:WESTGATE_TARGET_MINGW>
  $<$<BOOL:${TARGET_LINUX}>:WESTGATE_TARGET_LINUX>
  $<$<BOOL:${TARGET_APPLE}>:WESTGATE_TARGET_APPLE>
)
