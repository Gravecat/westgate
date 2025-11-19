# Very basic minimal CMake file for building fantasyname code as its own object.

cmake_minimum_required(VERSION 3.13)

add_library(fantasyname STATIC namegen.cpp)
target_compile_definitions(fantasyname PRIVATE
  HAVE_CXX14=1
  $<$<BOOL:${TARGET_WINDOWS}>:LOM_TARGET_WINDOWS>
  $<$<BOOL:${TARGET_MINGW}>:LOM_TARGET_MINGW>
  $<$<BOOL:${TARGET_LINUX}>:LOM_TARGET_LINUX>
  $<$<BOOL:${TARGET_APPLE}>:LOM_TARGET_APPLE>
)
