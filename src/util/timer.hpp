// util/timer.hpp -- A simple timer class for handling common timing functionality.

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
#include "core/pch.hpp" // precompiled header

#include <chrono>

namespace westgate {

class Timer
{
public:
                    Timer(unsigned int timer_length = 0);   // Constructor, sets up initial time stamp.
    unsigned int    elapsed() const;    // Checks how many milliseconds have passed since this timer began.
    bool            expired() const;    // Checks if this timer has reached its expiry length.
    void            randomize(unsigned int amount); // Randomizes the timer, by setting the start time later by a random amount between 1 and amount.
    void            reset(unsigned int timer_length = 0);   // Resets the timer to the time now. Exactly the same functionality as the constructor.

private:
    std::chrono::time_point<std::chrono::steady_clock>  start_time_;    // The time that this Timer started counting.
    unsigned int    timer_length_;  // The length of the timer before it expires, if any.
};

}   // namespace westgate
