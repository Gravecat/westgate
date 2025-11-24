// parser/world.cpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "parser/world.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"

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

    print(string("You travel to ") + (dir == Direction::UP || dir == Direction::DOWN ? "" : "the ") + Room::direction_name(dir) + ".\n");
    room_here->transfer(&player(), room_target);
    look(words_hashed, words);
}

} } }   // world, parser, westgate namespaces
