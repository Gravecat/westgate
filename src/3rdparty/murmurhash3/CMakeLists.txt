# Very basic minimal CMake file for building MurmurHash3 code as its own object.
# Raine "Gravecat" Simmons, 2025.

cmake_minimum_required(VERSION 3.10)

add_library(murmurhash3 STATIC MurmurHash3.cpp)
