// util/timer.cpp -- A simple timer class for handling common timing functionality.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

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
    int offset = 1;
    if (amount > 1) offset = rnd::get<int>(1, amount);
    start_time_ -= std::chrono::milliseconds(offset);
}

// Resets the timer to the time now. Exactly the same functionality as the constructor.
void Timer::reset(unsigned int timer_length)
{
    start_time_ = std::chrono::steady_clock::now();
    timer_length_ = timer_length;
}

}   // namespace westgate
