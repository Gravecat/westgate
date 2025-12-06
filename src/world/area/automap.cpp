// world/area/automap.cpp -- The Automap class automatically generates an ASCII map of the game world to display to the player.

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

#include "core/core.hpp"
#include "util/strx.hpp"
#include "world/area/automap.hpp"
#include "world/area/room.hpp"
#include "world/world.hpp"

using std::string;
using std::vector;

namespace westgate {

// Lookup table for converting Direction enums into X,Y vector directions (e.g. {-1, 0}).
const Vector3 Automap::direction_to_xyz_[10] = { {0,-1,0}, {1,-1,0}, {1,0,0}, {1,1,0}, {0,1,0}, {-1,1,0}, {-1,0,0}, {-1,-1,0}, {0,0,1}, {0,0,-1} };

// Generates a map centred on the specified Room.
vector<string> Automap::generate_map(Room* start_room)
{
    // Lookup tables for drawing room links in the game map vector.
    static const vector<int> link_offsets = { 0, -7, -6, 1, 8, 7, 6, -1, -8 };
    static const vector<char> link_symbols = { '\0', '|', '/', '-', '\\', '|', '/', '-', '\\' };

    // Get the room coordinates, and initialize the blank map.
    vector<string> game_map(7 * 7, "{0} ");

    // Determine which rooms are nearby.
    std::vector<Room*> nearby_rooms;
    std::vector<Vector3> nearby_coords;
    auto map_rooms = [&nearby_rooms, &nearby_coords](auto self, Room* room, int depth, Vector3 offset) {
        if (std::find(nearby_rooms.begin(), nearby_rooms.end(), room) != nearby_rooms.end()) return;
        if (!room->tag(RoomTag::Explored)) return;
        nearby_rooms.push_back(room);
        nearby_coords.push_back(offset);
        if (++depth > 3) return;
        for (unsigned int i = 1; i <= 10; i++)
        {
            const Direction dir = static_cast<Direction>(i);
            Room* new_room = room->get_link(dir);
            if (!new_room) continue;
            if (room->link_tag(dir, LinkTag::MapNoFollow)) continue;
            const int magnitude = (room->link_tag(dir, LinkTag::TripleLength) ? 6 : (room->link_tag(dir, LinkTag::DoubleLength) ? 4 : 2));
            self(self, new_room, depth + 1, offset + (direction_to_xyz_[i - 1] * magnitude));
        }
    };
    map_rooms(map_rooms, start_room, 0, {3, 3, 0});

    // Paint the rooms onto the map.
    for (size_t r = 0; r < nearby_rooms.size(); r++)
    {
        const Vector3 coord = nearby_coords.at(r);
        // Ignore any rooms that are outside of the space we're using to paint the map, or any that aren't on the same vertical level.
        if (coord.z != 0 || coord.x < 0 || coord.y < 0 || coord.x > 5 || coord.y > 5) continue;

        // Paint the room into the appropriate cell on the map.
        Room* room = nearby_rooms.at(r);
        const size_t vec_pos = (coord.x + (coord.y * 7));
        game_map.at(vec_pos) = (vec_pos == 24 ? "{R}@" : room->map_char());

        // Draw the room's exits onto the map.
        for (int i = 1; i <= 8; i++)
        {
            bool unfinished_link = false;
            unsigned int line_length = 1;
            const Direction dir = static_cast<Direction>(i);
            string link_colour = "{K}";
            if (!room->get_link(dir))
            {
                if (room->is_unfinished(dir, false))
                {
                    unfinished_link = true;
                    link_colour = "{r}";
                }
                else if (room->is_unfinished(dir, true))
                {
                    unfinished_link = true;
                    link_colour = "{R}";
                }
                else continue;
            }

            if (!unfinished_link) line_length = (room->link_tag(dir, LinkTag::TripleLength) ? 5 : (room->link_tag(dir, LinkTag::DoubleLength) ? 3 : 1));
            for (unsigned int l = 1; l <= line_length; l++)
            {
                Vector3 new_pos = coord + (direction_to_xyz_[i - 1] * static_cast<int>(l));
                if (new_pos.x < 0 || new_pos.y < 0 || new_pos.x > 6 || new_pos.y > 6) break;
                const unsigned int link_vec_pos = vec_pos + (link_offsets.at(i) * l);

                const char current_sym = game_map.at(link_vec_pos).at(game_map.at(link_vec_pos).size() - 1);
                const char new_sym = link_symbols.at(i);
                if (!unfinished_link && room->link_tag(dir, LinkTag::Openable) && !room->link_tag(dir, LinkTag::Open))
                {
                    if (room->link_tag(dir, LinkTag::AwareOfLock)) link_colour = "{R}";
                    else link_colour = "{y}";
                }
                
                // Check for overlapping / \ links, and turn them into an X.
                if (current_sym == 'X') continue;
                else if ((current_sym == '/' && new_sym == '\\') || (current_sym == '\\' && new_sym == '/') || current_sym == '+')
                {
                    game_map.at(link_vec_pos).pop_back();
                    game_map.at(link_vec_pos) += "X";
                }
                else game_map.at(link_vec_pos) = link_colour + string(1, new_sym);
            }
        }
    }

    // Combine the strings in the vector into single lines, and strip out excess colour tags.
    vector<string> map_out(7);
    for (int y = 0; y < 7; y++)
    {
        map_out.at(y) = " ";
        for (int x = 0; x < 7; x++)
            map_out.at(y) += game_map.at(x + (y * 7));
        map_out.at(y) = strx::flatten_tags(map_out.at(y) + "   " + "{0}");
    }

    // Crop any excess space from the map.
    while (map_out.size() && !map_out.at(0).compare(" {0}          "))
        map_out.erase(map_out.begin());
    while (map_out.size() && !map_out.at(map_out.size() - 1).compare(" {0}          "))
        map_out.erase(map_out.begin() + map_out.size() - 1);

    return map_out;
}

}   // namespace westgate
