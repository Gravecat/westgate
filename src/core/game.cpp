// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>

#include "3rdparty/rang/rang.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "world/codex.hpp"

namespace westgate {

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
    terminal::print("{R}Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi ultricies, felis et ultricies malesuada, quam felis bibendum nulla, in gravida nulla orci quis purus. Nullam sollicitudin id mi sed fermentum. Proin at dolor aliquam, fermentum arcu quis, commodo nisl. In a est elit. Proin egestas nibh eget viverra commodo. Aenean vitae tristique justo. Aliquam tincidunt aliquam neque, eu suscipit ante. Integer vel quam lacinia, viverra erat ac, tincidunt risus.");
    terminal::print("{Y}Cras luctus purus vitae semper vulputate. Aliquam congue lorem rhoncus pharetra commodo. Donec aliquam enim lacus, sit amet pulvinar purus tristique vel. Duis mattis mollis accumsan. Donec metus metus, mollis nec lectus ac, elementum efficitur enim. Nam sodales viverra purus, quis aliquet tortor lobortis quis. Aenean varius vel erat tincidunt faucibus. Aliquam eleifend nec justo sed lobortis. Morbi id maximus odio. Mauris id auctor arcu. Mauris mattis consectetur magna eget tincidunt. Maecenas fringilla felis sit amet velit tristique, sit amet consectetur odio vulputate. Cras tempus faucibus ex non egestas.");
    terminal::print("{G}In augue nulla, imperdiet eu faucibus vel, cursus elementum felis. Curabitur lacus ligula, pellentesque sit amet libero sit amet, tempor interdum justo. Duis eleifend nunc eu urna fringilla, eu molestie ipsum commodo. Suspendisse in purus dui. In hendrerit orci leo, quis consequat mi aliquet sit amet. Mauris neque risus, tempus sed nisi ac, varius accumsan erat. Pellentesque sagittis nulla ipsum, sed tristique erat fringilla at. Vestibulum ipsum sem, feugiat at congue sit amet, venenatis in arcu. Maecenas vel mi a est mollis accumsan. Mauris convallis justo interdum, pretium ligula ut, posuere tortor. Aenean sollicitudin sem ac auctor rhoncus. ");

    std::string input;
    while(true)
    {
        std::cout << '\n' << rang::style::reset << rang::fgB::green << "> ";
        std::getline(std::cin, input);
        std::cout << rang::style::reset << '\n';
        if (input.size()) process_input(input);
    }
}

// Sets up for a new game!
void Game::new_game() { }

// Processes input from the player.
void Game::process_input(const std::string &input)
{ terminal::print("{R}" + input); }

// A shortcut instead of using core().game()
Game& game() { return core().game(); }

}   // namespace westgate
