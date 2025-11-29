// world/area/automap.cpp -- The Automap class automatically generates an ASCII map of the game world to display to the player.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "trailmix/text/ansiutils.hpp"
#include "world/area/automap.hpp"
#include "world/area/room.hpp"
#include "world/world.hpp"

using std::string;
using std::vector;
using trailmix::math::Vector3;
using trailmix::text::ansi::flatten_tags;

namespace westgate {

// Adds a room to the vector coordinate cache.
void Automap::add_room_vec(uint32_t room_id, Vector3 vec)
{ room_vecs_.insert({vec, room_id}); }

// Retrieves a Room pointer for a specified coordinate in the world, or nullptr if it can't be found.
Room* Automap::find_room(Vector3 pos)
{
    auto result = room_vecs_.find(pos);
    if (result == room_vecs_.end()) return nullptr;
    else return world().find_room(result->second);
}

// Generates a map centred on the specified Room.
vector<string> Automap::generate_map(const Room* start_room)
{
    // Lookup tables for drawing room links in the game map vector.
    static const vector<int> link_offsets = { 0, -7, -6, 1, 8, 7, 6, -1, -8 };
    static const vector<char> link_symbols = { '\0', '|', '/', '-', '\\', '|', '/', '-', '\\' };

    // Get the room coordinates, and initialize the blank map.
    const Vector3 room_coords = start_room->coords();
    vector<string> game_map(7 * 7, "{0} ");

    // Paint the rooms onto the map.
    for (int room_x = 0; room_x <= 2; room_x++)
    {
        for (int room_y = 0; room_y <= 2; room_y++)
        {
            // Look up the room, and paint it into the appropriate cell on the map if it exists and is explored.
            const unsigned int vec_pos = ((room_x * 2)) + (room_y * 14) + 8;
            const Vector3 coord = room_coords + Vector3(room_x - 1, room_y - 1, 0);
            Room* room = find_room(coord);
            if (!room || !room->tag(RoomTag::Explored)) continue;
            game_map.at(vec_pos) = (room_x == 1 && room_y == 1 ? "{R}@" : room->map_char());

            // Draw the room's exits onto the map.
            for (int i = 1; i <= 8; i++)
            {
                bool unfinished_link = false;
                if (!room->get_link(static_cast<Direction>(i)))
                {
                    if (room->is_unfinished(static_cast<Direction>(i))) unfinished_link = true;
                    else continue;
                }
                const unsigned int link_vec_pos = vec_pos + link_offsets.at(i);
                const char current_sym = game_map.at(link_vec_pos).at(game_map.at(link_vec_pos).size() - 1);
                const char new_sym = link_symbols.at(i);
                
                // Check for overlapping / \ links, and turn them into an X.
                if (current_sym == 'X') continue;
                else if ((current_sym == '/' && new_sym == '\\') || (current_sym == '\\' && new_sym == '/'))
                {
                    game_map.at(link_vec_pos).pop_back();
                    game_map.at(link_vec_pos) += "X";
                }
                else game_map.at(link_vec_pos) = (unfinished_link ? "{r}" : "{K}") + string(1, link_symbols.at(i));
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
        map_out.at(y) = flatten_tags(map_out.at(y) + "   " + "{0}");
    }

    // Crop any excess space from the map.
    while (map_out.size() && !map_out.at(0).compare(" {0}          "))
        map_out.erase(map_out.begin());
    while (map_out.size() && !map_out.at(map_out.size() - 1).compare(" {0}          "))
        map_out.erase(map_out.begin() + map_out.size() - 1);

    return map_out;
}

}   // namespace westgate
