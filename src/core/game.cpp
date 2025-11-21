// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>
#include <iostream>

#include "3rdparty/rang/rang.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/file/binpath.hpp"
#include "world/area/region.hpp"
#include "world/codex.hpp"

namespace westgate {

// Constructor, sets up the game manager.
Game::Game() : codex_ptr_(nullptr), region_ptr_(nullptr), save_id_(-1) { }

// Destructor, cleans up attached classes.
Game::~Game()
{
    codex_ptr_.reset(nullptr);
    region_ptr_.reset(nullptr);
}

// Starts the game, in the form of a title screen followed by the main game loop.
void Game::begin()
{
    // Except there is no title screen yet. That'll come later.

    codex_ptr_ = std::make_unique<Codex>();
    create_world();
    new_game();
    main_loop();
}

// Returns a reference to the Codex object.
Codex& Game::codex() const
{
    if (!codex_ptr_) throw std::runtime_error("Attempt to access null Codex pointer!");
    return *codex_ptr_;
}

// Loads the static YAML data and generates a binary save file for the game world.
void Game::create_world()
{
    // This can be replaced with something better later.
    terminal::print();
    terminal::print("{Y}Generating game world from static data...");

    // Create a game saves folder, if one doesn't already exist.
    const std::filesystem::path userdata_saves_path = BinPath::game_path("userdata/saves");
    if (!std::filesystem::is_directory(userdata_saves_path)) std::filesystem::create_directory(userdata_saves_path);

    // Right now, we're hard-coding save slot 0. Later in development, we'll let the user choose multiple save slots.
    save_id_ = 0;

    // This early in development, we're gonna just delete the save folder each time. It'll become more permanent later.
    const std::filesystem::path save_dir = userdata_saves_path.string() + "/" + std::to_string(save_id_);
    std::filesystem::remove_all(save_dir);
    std::filesystem::create_directory(save_dir);

    // Determine how many region files are in the game's data files.
    const std::filesystem::path regions_folder = core().datafile("world/regions");
    std::vector<std::filesystem::path> regions;
    for (const auto& file : std::filesystem::directory_iterator(regions_folder))
        if (file.is_regular_file()) regions.push_back(file.path().filename());

    // One at a time, load each region into memory.
    for (unsigned int i = 0; i < regions.size(); i++)
    {
        terminal::print("{C}Processing region file {G}" + std::to_string(i + 1) + " {C}of {G}" + std::to_string(regions.size()) + "{C}...");
        std::filesystem::path region_file = regions.at(i);
        std::unique_ptr<Region> new_region = std::make_unique<Region>();
        new_region->load_from_gamedata(region_file.string());
        new_region->save(save_id_);
    }
    terminal::print("{Y}World generation complete!");
    terminal::print();
}

// Shuts things down cleanly and exits the game.
void Game::leave_game() { core().destroy_core(EXIT_SUCCESS); }

// brøether, may i have the lööps
void Game::main_loop()
{
    terminal::print("{R}Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi ultricies, felis et ultricies malesuada, quam felis {M}1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890 {R}bibendum nulla, in gravida nulla orci quis purus. Nullam sollicitudin id mi sed fermentum. Proin at dolor aliquam, fermentum arcu quis, commodo nisl. In a est elit. Proin egestas nibh eget viverra commodo. Aenean vitae tristique justo. Aliquam tincidunt aliquam neque, eu suscipit ante. Integer vel quam lacinia, viverra erat ac, tincidunt risus.");
    terminal::print();
    terminal::print("{Y}Cras luctus purus vitae semper vulputate. Aliquam congue lorem rhoncus pharetra commodo. Donec aliquam enim lacus, sit amet pulvinar purus tristique vel. Duis mattis mollis accumsan. Donec metus metus, mollis nec lectus ac, elementum efficitur enim. Nam sodales viverra purus, quis aliquet tortor lobortis quis. Aenean varius vel erat tincidunt faucibus. Aliquam eleifend nec justo sed lobortis. Morbi id maximus odio. Mauris id auctor arcu. Mauris mattis consectetur magna eget tincidunt. Maecenas fringilla felis sit amet velit tristique, sit amet consectetur odio vulputate. Cras tempus faucibus ex non egestas.");
    terminal::print("{G}In augue nulla, imperdiet eu faucibus vel, cursus elementum felis. Curabitur lacus ligula, pellentesque sit amet libero sit amet, tempor interdum justo. Duis eleifend nunc eu urna fringilla, eu molestie ipsum commodo. Suspendisse in purus dui. In hendrerit orci leo, quis consequat mi aliquet sit amet. Mauris neque risus, tempus sed nisi ac, varius accumsan erat. Pellentesque sagittis nulla ipsum, sed tristique erat fringilla at. Vestibulum ipsum sem, feugiat at congue sit amet, venenatis in arcu. Maecenas vel mi a est mollis accumsan. Mauris convallis justo interdum, pretium ligula ut, posuere tortor. Aenean sollicitudin sem ac auctor rhoncus. ");

    std::string input;
    while(true)
    {
        std::cout << '\n' << rang::style::reset << rang::fgB::green << "> ";
        input = "";
        do { std::getline(std::cin, input); } while(!input.size());
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
