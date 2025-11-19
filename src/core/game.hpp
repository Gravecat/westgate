// core/game.hpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace lom {

class Game {
public:
            Game();         // Constructor, sets up the game manager.
            ~Game();        // Destructor, cleans up attached classes.
    void    begin();        // Starts the game, in the form of a title screen followed by the main game loop.
    void    leave_game();   // Shuts things down cleanly and exits the game.
    void    process_input(const std::string &input);    // Processes input from the player.

private:
    void    main_loop();    // brøether, may i have the lööps
    void    new_game();     // Sets up for a new game!
};

Game&   game(); // A shortcut instead of using core().game()

}   // namespace lom
