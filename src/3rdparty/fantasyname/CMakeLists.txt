# Very basic minimal CMake file for building fantasyname code as its own object.

cmake_minimum_required(VERSION 3.13)

add_library(fantasyname STATIC namegen.cpp)
target_compile_definitions(fantasyname PRIVATE HAVE_CXX14=1)
