// actions/cheats.cpp -- Cheating/development/testing commands.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "actions/cheats.hpp"
#include "trailmix/text/manipulation.hpp"
#include "world/time/time-weather.hpp"
#include "world/world.hpp"

using std::to_string;
using trailmix::text::manipulation::str_toupper;
using westgate::terminal::print;

namespace westgate::actions::cheats {

// Hashes words into integers.
void hash(PARSER_FUNCTION)
{
    if (words.size() < 2)
    {
        print("{Y}Please specify a word to be hashed.");
        return;
    }
    print("The hashed version of {C}" + words.at(1) + " {w}is {C}" + to_string(words_hashed.at(1)));
}

// Retrieves a specified weather-map description.
void weather_desc(PARSER_FUNCTION)
{ PARSER_NO_HASHED
    if (words.size() < 2)
    {
        print("{Y}Please specify a weather string to print.");
        return;
    }
    print("{K}" + world().time_weather().string_map(str_toupper(words.at(1))));
}

}   // namespace westgate::actions::cheats
