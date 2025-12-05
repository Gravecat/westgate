// core/game.hpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

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

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {

class Player;   // defined in world/entity/player.hpp
class World;    // defined in world/world.hpp

class Game {
public:
            Game();         // Constructor, sets up the game manager.
            ~Game();        // Destructor, cleans up attached classes.
    void    begin();        // Starts the game, in the form of a title screen followed by the main game loop.
    void    leave_game();   // Shuts things down cleanly and exits the game.
    Player& player() const; // Returns a reference to the Player object.
    void    save(bool chatty = true);   // Save the game, if there's a game in progress.
    int     save_slot() const;  // Returns the currently-used saved game slot.
    void    set_player(Player* player_ptr); // Sets the Player pointer. Use with caution.
    World&  world() const;  // Returns a reference to the World object.

private:
    static constexpr uint32_t   MISC_DATA_SAVE_VERSION = 6; // The version of the misc data file in save files. Changing this will make save files incompatible.

    Player* player_ptr_;    // Pointer to the player-character object. Ownership of the object lies with the Room they're in.
    int     save_id_;       // The current saved-game ID (or -1 for none).
    std::unique_ptr<World>  world_ptr_;     // The World object, which handles the state of the game world as well as the static data.

    void    load_game(int save_slot);   // Loads an existing saved game.
    void    main_loop();        // brøether, may i have the lööps
    void    new_game(int starting_region, std::string_view starting_room);    // Sets up for a new game!
    void    save_misc_data();   // Writes a misc save file, which contains everything that isn't in the region saves.
    void    title_screen();     // Every game needs a title screen!
};

Game&   game(); // A shortcut instead of using core().game()

}   // namespace westgate
