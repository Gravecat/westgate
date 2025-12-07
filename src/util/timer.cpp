// util/timer.cpp -- A simple timer class for handling common timing functionality.

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

#include "util/random.hpp"
#include "util/timer.hpp"

namespace westgate {

// Constructor, sets up initial time stamp.
Timer::Timer(unsigned int timer_length) : start_time_(std::chrono::steady_clock::now()), timer_length_(timer_length) { }

// Checks how many milliseconds have passed since this timer began.
unsigned int Timer::elapsed() const
{ return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_).count(); }

// Checks if this timer has reached its expiry length.
bool Timer::expired() const
{
    if (timer_length_ && elapsed() >= timer_length_) return true;
    else return false;
}

// Randomizes the timer, by setting the start time later by a random amount between 1 and amount.
void Timer::randomize(unsigned int amount)
{
    int offset = (amount > 1 ? rnd::get<int>(1, amount) : 1);
    start_time_ -= std::chrono::milliseconds(offset);
}

// Resets the timer to the time now. Exactly the same functionality as the constructor.
void Timer::reset(unsigned int timer_length)
{
    start_time_ = std::chrono::steady_clock::now();
    timer_length_ = timer_length;
}

}   // namespace westgate
