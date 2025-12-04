// world/time/timing.hpp -- Constants that define the amount of time it takes to perform various actions in the game world. All the timing values in this file
// are in seconds.

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
#include "core/pch.hpp"

namespace westgate {
namespace timing {

static constexpr int    MINUTE =    60;
static constexpr int    HOUR =      60 * 60;
static constexpr int    DAY =       60 * 60 * 24;

static constexpr int    TIME_TO_MOVE =  30;         // The time it takes to move from one Room to another.
static constexpr int    TIME_TO_WAIT =  5 * MINUTE; // The base time to wait if the "wait" command is used without specifying a time.

} }
