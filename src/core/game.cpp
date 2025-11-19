// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/game.hpp"
#include "world/codex.hpp"

namespace lom {

// Constructor, sets up the game manager.
Game::Game() : codex_ptr_(nullptr) { }

// Destructor, cleans up attached classes.
Game::~Game()
{ codex_ptr_.reset(nullptr); }

// Starts the game, in the form of a title screen followed by the main game loop.
void Game::begin()
{
    // Except there is no title screen yet. That'll come later.

    codex_ptr_ = std::make_unique<Codex>();
    new_game();
    main_loop();
}

// Returns a reference to the Codex object.
Codex& Game::codex() const
{
    if (!codex_ptr_) throw std::runtime_error("Attempt to access null Codex pointer!");
    return *codex_ptr_;
}

// Shuts things down cleanly and exits the game.
void Game::leave_game() { core().destroy_core(EXIT_SUCCESS); }

// brøether, may i have the lööps
void Game::main_loop()
{
    std::cout << fgB::cyan << "Hello, world!" << EOL;
    std::string input;
    while(true)
    {
        std::getline(std::cin, input);
        process_input(input);
    }
}

// Sets up for a new game!
void Game::new_game() { }

// Processes input from the player.
void Game::process_input(const std::string &input)
{ std::cout << std::endl << fgB::green << input << std::endl << EOL; }

// A shortcut instead of using core().game()
Game& game() { return core().game(); }

}   // namespace lom
