// core/game.hpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {

class Codex;    // defined in world/codex.hpp
class Player;   // defined in world/entity/player.hpp
class Region;   // defined in world/area/region.hpp

class Game {
public:
                    Game();         // Constructor, sets up the game manager.
                    ~Game();        // Destructor, cleans up attached classes.
    void            begin();        // Starts the game, in the form of a title screen followed by the main game loop.
    Codex&          codex() const;  // Returns a reference to the Codex object.
    void            leave_game();   // Shuts things down cleanly and exits the game.
    Player&         player() const; // Returns a reference to the Player object.
    void            save(bool chatty);  // Save the game, if there's a game in progress.
    void            set_player(Player* player_ptr); // Sets the Player pointer. Use with caution.

private:
    static constexpr uint32_t   METADATA_SAVE_VERSION = 1;  // The version of the metadata file in save files. Changing this will make save files incompatible.

    std::unique_ptr<Codex>  codex_ptr_;     // The Codex object, which stores all the static game data in memory, and generates copies of said data.
    Player*                 player_ptr_;    // Pointer to the player-character object. Ownership of the object lies with the Room they're in.
    std::unique_ptr<Region> region_ptr_;    // The currently-loaded Region, the area of the game world the player is interacting with.
    int     save_id_;       // The current saved-game ID (or -1 for none).

    void    create_world();     // Loads the static YAML data and generates a binary save file for the game world.
    void    load_game(int save_slot);   // Loads an existing saved game.
    void    main_loop();        // brøether, may i have the lööps
    void    new_game(const uint32_t starting_region, const std::string& starting_room); // Sets up for a new game!
    void    save_metadata();    // Saves a metadata save file, which contains basic info like the current region and save file version.
    void    title_screen();     // Every game needs a title screen!
};

Game&   game(); // A shortcut instead of using core().game()

}   // namespace westgate
