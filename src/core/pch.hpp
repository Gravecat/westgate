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
