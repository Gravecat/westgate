// parser/world.cpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "parser/world.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"

using std::string;
using westgate::terminal::print;

namespace westgate {
namespace parser {
namespace world {

// Look around you. Just look around you.
void look(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    player().parent_room()->look();
}

// Travels in a specific direction.
void travel(PARSER_FUNCTION)
{
    uint32_t direction_hash = words_hashed.at(0);
    if (direction_hash == 93100650 || direction_hash == 1337450370 || direction_hash == 1908976648) // go, move, travel
    {
        if (words_hashed.size() < 2)
        {
            print("{Y}Please specify a direction to travel.");
            return;
        }
        else direction_hash = words_hashed.at(1);
    }
    Direction dir = parser::parse_direction(direction_hash);
    if (dir == Direction::NONE)
    {
        print("{Y}I don't understand what you mean. Please specify a cardinal direction, or up/down.");
        return;
    }

    Room* room_here = player().parent_room();
    Room* room_target = room_here->get_link(dir);
    if (!room_target)
    {
        print("{Y}You can't travel in that direction.");
        return;
    }

    print(string("You travel to ") + (dir == Direction::UP || dir == Direction::DOWN ? "" : "the ") + Room::direction_name(dir) + ".");
    room_here->transfer(&player(), room_target);
    westgate::world().time_weather().pass_time(TimeWeather::TIME_TO_MOVE);
    look(words_hashed, words);
}

// Waits or rests for a specified period of time.
void wait(PARSER_FUNCTION)
{
    if (words_hashed.size() < 2)
    {
        print("Time passes...");
        westgate::world().time_weather().pass_time(5 * 60, true);
        return;
    }
    if (words_hashed.size() < 3)
    {
        print("{Y}Please specify exactly how long you want to wait (e.g. {G}wait 35 minutes{Y}).");
        return;
    }
    const int original_amount = parser::parse_number(words.at(1));
    int amount = original_amount;
    if (amount == INT_MIN)
    {
        print("{Y}I don't understand that. Please specify exactly how long you want to wait, using numerics for the digits (e.g. {G}wait 35 minutes{Y}).");
        return;
    }
    if (amount < 0)
    {
        print("{Y}Don't be ridiculous.");
        return;
    }
    std::string time_str;
    switch(words_hashed.at(2))
    {
        case 1296922301: case 3652255926: time_str = "second"; break;
        case 4000051627: case 1409649807: time_str = "minute"; amount *= 60; break;
        case 3692426535: case 1910453879: time_str = "hour"; amount *= 60 * 60; break;
        case 239299983: case 3336843723: time_str = "day"; amount *= 60 * 60 * 24; break;
        default:
            print("{Y}I don't understand. Please specify how long you want to wait in {G}seconds{Y}, {G}minutes{Y}, {G}hours{Y} or {G}days{Y}.");
            return;
    }
    print("You prepare to wait for " + stringutils::number_to_text(original_amount) + " " + time_str + (original_amount > 1 ? "s." : "."));
    westgate::world().time_weather().pass_time(amount, true);
}

} } }   // world, parser, westgate namespaces
