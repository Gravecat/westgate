# 3rdparty/rapidyaml/CMakeLists.txt -- Very basic minimal CMake file for building Rapid YAML code as its own object.

# This file is dedicated to the public domain under CC0 1.0 Universal. To the extent possible under law,
# Raine "Gravecat" Simmons has waived all copyright and related or neighbouring rights to this file.
# See https://creativecommons.org/publicdomain/zero/1.0/ for details.

cmake_minimum_required(VERSION 3.10)

add_library(rapidyaml STATIC rapidyaml.cpp)
