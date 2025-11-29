// actions/world-interaction.cpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "actions/world-interaction.hpp"
#include "core/core.hpp"
#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "trailmix/math/vector3.hpp"
#include "trailmix/text/conversion.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"
#include "world/time/timing.hpp"
#include "world/world.hpp"

using std::string;
using trailmix::math::Vector3;
using trailmix::text::conversion::number_to_text;
using westgate::terminal::print;

namespace westgate::actions::world_interaction {

// Look around you. Just look around you.
void look(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    player().parent_room()->look();
}

// Attempts to open or close a door or similar.
void open_close(PARSER_FUNCTION)
{ PARSER_NO_WORDS
    const bool open = (words_hashed.at(0) == 21229531);
    const string open_close = (open ? "open" : "close");
    const string open_closed = (open ? "open" : "closed");

    if (words_hashed.size() < 2)
    {
        print("{Y}Please specify a direction to " + open_close + " something.");
        return;
    }
    Direction dir = parser::parse_direction(words_hashed.at(1));
    if (dir == Direction::NONE)
    {
        print("{Y}I don't understand. Please specify a direction to " + open_close + " something.");
        return;
    }

    Room* room = player().parent_room();
    if (!room->get_link(dir))
    {
        print("{Y}There isn't anything to " + open_close + " in that direction.");
        return;
    }
    if (!room->link_tag(dir, LinkTag::Openable))
    {
        print("{Y}That isn't something you can " + open_close + "!");
        return;
    }
    const bool is_open = room->link_tag(dir, LinkTag::Open);
    if ((open && is_open) || (!open && !is_open))
    {
        print("{Y}It's already " + open_closed + ".");
        return;
    }
    if (room->link_tag(dir, LinkTag::Locked) || room->link_tag(dir, LinkTag::Permalock))
    {
        print("{Y}You try to open the " + room->door_name(dir) + ", but it's locked.");
        return;
    }

    world().open_close_no_checks(room, dir, open);
    print("You " + open_close + " the " + room->door_name(dir) + ".");
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
        if (room_here->is_unfinished(dir)) print("{Y}You can't travel in that direction; that part of the game is not yet finished.");
        else print("{Y}You can't travel in that direction.");
        return;
    }

    if ((room_here->link_tag(dir, LinkTag::Locked) || room_here->link_tag(dir, LinkTag::Permalock)) && !room_here->link_tag(dir, LinkTag::Open))
    {
        print("{Y}You can't go that way, the " + room_here->door_name(dir) + " is locked.");
        return;
    }

    if (room_here->link_tag(dir, LinkTag::Openable))
    {
        if (!room_here->link_tag(dir, LinkTag::Open))
        {
            print("{B}(first opening the " + room_here->door_name(dir) + ")");
            world().open_close_no_checks(room_here, dir, true);
        }
    }

    print(string("You travel ") + (dir == Direction::UP || dir == Direction::DOWN ? "" : "to the ") + Room::direction_name(dir) + ".");
    room_here->transfer(&player(), room_target);
    world().time_weather().pass_time(timing::TIME_TO_MOVE);
    look(words_hashed, words);

    // If we're on a debug build, ensure the room coordinates align correctly. There may be edge cases eventually, but we can make a special LinkTag for that
    // if and when it happens. This is computationally cheap, but we're restricting it to debug builds anyway, to be efficient.
#ifdef WESTGATE_BUILD_DEBUG
    const Vector3 old_coord = room_here->coords();
    const Vector3 new_coord = room_target->coords();
    Vector3 expected_coord = old_coord;
    if (dir == Direction::UP) expected_coord.z++;
    else if (dir == Direction::DOWN) expected_coord.z--;
    else
    {
        if (dir == Direction::NORTH || dir == Direction::NORTHEAST || dir == Direction::NORTHWEST) expected_coord.y--;
        else if (dir == Direction::SOUTH || dir == Direction::SOUTHWEST || dir == Direction::SOUTHEAST) expected_coord.y++;
        if (dir == Direction::EAST || dir == Direction::NORTHEAST || dir == Direction::SOUTHEAST) expected_coord.x++;
        else if (dir == Direction::WEST || dir == Direction::NORTHWEST || dir == Direction::SOUTHWEST) expected_coord.x--;
    }
    if (new_coord != expected_coord) core().nonfatal("Moved in an unexpected direction! Old coordinates were [" + old_coord.string() + "], new coords are [" +
        new_coord.string() + "], expected coords were [" + expected_coord.string() + "]", Core::CORE_WARN);
#endif
}

// Waits or rests for a specified period of time.
void wait(PARSER_FUNCTION)
{
    if (words_hashed.size() < 2)
    {
        print("Time passes...");
        world().time_weather().pass_time(timing::TIME_TO_WAIT, true);
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
    print("You prepare to wait for " + number_to_text(original_amount) + " " + time_str + (original_amount > 1 ? "s" : "") +
        ". Time passes...");
    world().time_weather().pass_time(amount, true);
}

}   // westgate::actions::world_interaction namespace
