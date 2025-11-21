// world/area/room.hpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "world/entity/entity.hpp"

namespace westgate {

class FileWriter;   // defined in util/file/filewriter.hpp

class Room {
public:
                        Room(const std::string& new_id);    // Creates a blank Room with default values.
    const std::string&  desc() const;   // Retrieves the description of this Room.
    uint32_t            id() const;     // Retrieves the hashed ID of this Room.
    const std::string&  name(bool full_name) const; // Retrieves the name of this Room.
    void                save(FileWriter* file);     // Saves this Room to a specified save file. Should only be called by a parent Region.
    void                set_desc(const std::string& new_desc);  // Sets the description of this Room.
    void                set_name(const std::string& new_full_name, const std::string& new_short_name);  // Sets the name of this Room.
    void                transfer(Entity* entity_ptr, Room* room_ptr);   // Transfers a specified Entity from this Room to a target Room.

protected:
    std::vector<std::unique_ptr<Entity>>    entities_;  // Entities that exist within this Room.

private:
    static constexpr int    ROOM_SAVE_VERSION =   1;    // The expected version for saving/loading binary game data.

    std::string desc_;      // The text description of this Room, as shown to the player.
    uint32_t    id_;        // The Room's unique hashed ID.
    std::string id_str_;    // The Room's unique text ID.
    std::string name_[2];   // The name of this Room, both in full-name and short-name form.
};

}   // namespace westgate
