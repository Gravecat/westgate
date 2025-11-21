# Very basic minimal CMake file for building Rapid YAML code as its own object.
# Raine "Gravecat" Simmons, 2025.

cmake_minimum_required(VERSION 3.10)

add_library(rapidyaml STATIC rapidyaml.cpp)
