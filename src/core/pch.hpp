// core/pch.hpp -- Precompiled global header, that includes things which are used pretty much everywhere in this project.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once

#include <climits>      // Integer limit macros like UINT_MAX
#include <cstddef>      // Primarily included here for size_t
#include <cstdint>      // Frequently-used integer defintions like uint32_t
#include <cstdlib>      // EXIT_FAILURE, EXIT_SUCCESS macro constants
#include <memory>       // std::unique_ptr and std::move are used frequently
#include <stdexcept>    // std::runtime_error is used frequently to throw error exceptions
#include <string>       // std::string is used in almost every part of this project
#include <vector>       // std::vector is used frequently enough to belong here too

// Typedefs for hashed strings, and a size_t analogue that we'll use for reading/writing data in binary saved game files.
namespace westgate {
using hash_wg = uint32_t;   // Hashed strings are always 32-bit unsigned integers.

// Sizes of elements like strings or vectors, when written into a binary save file. We want to check the size of size_t, and go either 16-bit if size_t is
// 16-bit width, or 32-bit if it's larger. There is no reason for this game to ever need size_wg to be 64-bit, and by avoiding 64-bit, we make saved game
// files much more compatible between 64- and 32-bit computers. By allowing size_wg to be 16-bit if needed, we avoid a 32-bit saved game file possibly loading
// in an utterly corrupted state on a 16-bit platform.
#if SIZE_MAX == UINT16_MAX
using size_wg = uint16_t;
#elif SIZE_MAX >= UINT32_MAX
using size_wg =  uint32_t;
#else
#error Unsupported size_t size
#endif
}
