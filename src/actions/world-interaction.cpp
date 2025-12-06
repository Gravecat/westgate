// actions/world-interaction.cpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

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

#include "actions/world-interaction.hpp"
#include "core/core.hpp"
#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "util/strx.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"
#include "world/time/timing.hpp"
#include "world/world.hpp"

using std::string;
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
        if (room->is_unfinished(dir, true))
        {
            if (open) print("{Y}You try to open it, but it's locked.");
            else print("{Y}It's already closed.");
        }
        else print("{Y}There isn't anything to " + open_close + " in that direction.");
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
        room->set_link_tag(dir, LinkTag::AwareOfLock);
        return;
    }

    world().open_close_lock_unlock_no_checks(room, dir, (open ? World::OpenCloseLockUnlock::OPEN : World::OpenCloseLockUnlock::CLOSE), &player());
    print("You " + open_close + " the " + room->door_name(dir) + ".");
}

// Travels in a specific direction.
void travel(PARSER_FUNCTION)
{
    hash_wg direction_hash = words_hashed.at(0);
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
        if (room_here->is_unfinished(dir, false)) print("{Y}You can't travel in that direction; that part of the game is not yet finished.");
        else if (room_here->is_unfinished(dir, true)) print("{Y}You can't go that way, it's locked.");
        else print("{Y}You can't travel in that direction.");
        return;
    }

    if ((room_here->link_tag(dir, LinkTag::Locked) || room_here->link_tag(dir, LinkTag::Permalock)) && !room_here->link_tag(dir, LinkTag::Open))
    {
        print("{Y}You can't go that way, the " + room_here->door_name(dir) + " is locked.");
        room_here->set_link_tag(dir, LinkTag::AwareOfLock);
        return;
    }

    if (room_here->link_tag(dir, LinkTag::Openable))
    {
        if (!room_here->link_tag(dir, LinkTag::Open))
        {
            print("{B}(first opening the " + room_here->door_name(dir) + ")");
            world().open_close_lock_unlock_no_checks(room_here, dir, World::OpenCloseLockUnlock::OPEN, &player());
        }
    }

    print(string("You travel ") + (dir == Direction::UP || dir == Direction::DOWN ? "" : "to the ") + Room::direction_name(dir) + ".");
    room_here->transfer(&player(), room_target);
    world().time_weather().pass_time(timing::TIME_TO_MOVE);
    look(words_hashed, words);
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
    string time_str;
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
    print("You prepare to wait for " + strx::number_to_text(original_amount) + " " + time_str + (original_amount > 1 ? "s" : "") +
        ". Time passes...");
    world().time_weather().pass_time(amount, true);
}

}   // westgate::actions::world_interaction namespace
