// util/math/vector3.hpp -- The Vector3 struct provides a simple way of storing and sharing 3D coordinates.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <initializer_list>

namespace westgate {

// Simple three-dimensional integer coordinate struct.
struct Vector3
{
    Vector3() : x(0), y(0), z(0) { }
    Vector3(int32_t vx, int32_t vy, int32_t vz) : x(vx), y(vy), z(vz) { }
    Vector3(std::initializer_list<int32_t> list)
    {
        auto it = list.begin();
        x = (it != list.end()) ? *it++ : 0;
        y = (it != list.end()) ? *it++ : 0;
        z = (it != list.end()) ? *it : 0;
    }

    Vector3     operator+(const Vector3 &other) const { return { x + other.x, y + other.y, z + other.z }; }
    Vector3     operator-(const Vector3 &other) const { return { x - other.x, y - other.y, z - other.z }; }
    Vector3     operator/(const Vector3 &other) const { return { x / other.x, y / other.y, z / other.z }; }
    Vector3     operator*(const Vector3 &other) const { return { x * other.x, y * other.y, z * other.z }; }
    Vector3     operator%(const Vector3 &other) const { return { x % other.x, y % other.y, z % other.z }; }
    bool        operator==(const Vector3 &other) const { return (x == other.x && y == other.y && z == other.z); }
    bool        operator!=(const Vector3 &other) const { return (x != other.x || y != other.y || z != other.z); }
    bool        operator!() const { return (x == 0 && y == 0 && z == 0); }
    bool        operator>(const Vector3 &other) const { return (x > other.x && y > other.y && z > other.z); }
    bool        operator<(const Vector3 &other) const { return (x < other.x || y < other.y || z < other.z); }
    explicit    operator bool() const { return (x != 0 || y != 0 || z != 0); }

    template<typename T> Vector3    operator/(const T other) const { return {x / other, y / other, z / other}; }
    template<typename T> Vector3    operator*(const T other) const { return {x * other, y * other, z * other}; }
    template<typename T> Vector3    operator%(const T other) const { return {x % other, y % other, z % other}; }

    int32_t x, y, z;
};

inline void hash_combine(std::size_t& seed, std::size_t h) noexcept
{ seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2); }

}   // namespace westgate

// specialise std::hash
namespace std {
using westgate::hash_combine;
template <> struct hash<westgate::Vector3>
{
    std::size_t operator()(const westgate::Vector3& v) const noexcept
    {
        std::size_t seed = 0;
        hash_combine(seed, std::hash<int32_t>{}(v.x));
        hash_combine(seed, std::hash<int32_t>{}(v.y));
        hash_combine(seed, std::hash<int32_t>{}(v.z));
        return seed;
    }
};
}   // namespace std
