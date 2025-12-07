// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

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

#include <filesystem>

#include "cmake/version.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "util/filex.hpp"
#include "util/strx.hpp"
#include "util/timer.hpp"
#include "world/area/region.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"
#include "world/world.hpp"

using std::runtime_error;
using std::string;
using std::string_view;
using std::to_string;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Constructor, sets up the game manager.
Game::Game() : player_ptr_(nullptr), save_id_(-1), world_ptr_(nullptr) { }

// Destructor, cleans up attached classes.
Game::~Game() { world_ptr_.reset(nullptr); }

// Starts the game, in the form of a title screen followed by the main game loop.
void Game::begin()
{
    world_ptr_ = std::make_unique<World>();
    title_screen();
    print();
    player_ptr_->parent_room()->look();
    main_loop();
}

// Returns a reference to the World object.
World& Game::world() const
{
    if (!world_ptr_) throw runtime_error("Attempt to access null World pointer!");
    return *world_ptr_;
}

// Shuts things down cleanly and exits the game.
void Game::leave_game() { core().destroy_core(EXIT_SUCCESS); }

// Loads an existing saved game.
void Game::load_game(int save_slot)
{
    Timer load_timer;

    player_ptr_ = nullptr;
    const fs::path save_path = filex::game_path("userdata/saves/" + to_string(save_slot));
    if (!fs::exists(save_path))
    {
        print("{R}Saved game file cannot be located.");
        core().destroy_core(EXIT_SUCCESS);
    }

    // Load the misc data file.
    const fs::path misc_path = filex::merge_paths(save_path.string(), "savedata.wg");
    if (!fs::exists(misc_path)) throw runtime_error("Could not locate saved game data!");
    std::unique_ptr<FileReader> file = std::make_unique<FileReader>(misc_path.string());

    // Check the misc data headers and version.
    if (!file->check_header()) throw runtime_error("Invalid save data header!");
    if (const unsigned int misc_version = file->read_data<unsigned int>();
        misc_version != MISC_DATA_SAVE_VERSION) FileReader::standard_error("Invalid save data version", misc_version, MISC_DATA_SAVE_VERSION);
    if (file->read_string() != "MISC_DATA") throw runtime_error("Invalid save data header!");

    // Check what Region the player is in.
    const int current_region = file->read_data<int>();

    // Load the time/weather data.
    world_ptr_->time_weather().load_data(file.get());

    // Finally, check the footer before closing the file.
    if (!file->check_footer()) throw runtime_error("Invalid save data footer!");
    file.reset(nullptr);

    // Load the Region that contains the Player object.
    world_ptr_->load_region(current_region);

    print("{c}Saved game loaded successfully!");
    core().log("Saved game loaded in " + strx::ftos(load_timer.elapsed() / 1000.0f, 3) + " seconds.");
}

// brøether, may i have the lööps
void Game::main_loop() { while(true) { parser::process_input(terminal::get_input()); } }

// Sets up for a new game!
void Game::new_game(int starting_region, string_view starting_room)
{
    Timer new_game_timer;

    // Create the new region delta save files.
    world_ptr_->create_region_saves(save_id_);

    // Create the player character, assign them to a starting room, then transfer ownership.
    auto player = std::make_unique<Player>(nullptr);
    Room* start_room = world_ptr_->find_room(starting_room, starting_region);
    start_room->add_entity(std::move(player));

    // Save the game silently, to store the player character.
    save(false);

#ifdef WESTGATE_BUILD_DEBUG
    core().log("New game initialized in " + strx::ftos(new_game_timer.elapsed() / 1000.0f, 3) + " seconds.");
#endif
}

// Returns a reference to the Player object.
Player& Game::player() const { return *player_ptr_; }

// Save the game, if there's a game in progress.
void Game::save(bool chatty)
{
    if (chatty) print("{c}Saving the game...", false);
    world_ptr_->save(save_id_);
    save_misc_data();
    if (chatty) print(" Done!");
}

// Writes a misc save file, which contains everything that isn't in the region saves.
void Game::save_misc_data()
{
    const fs::path save_path = filex::game_path("userdata/saves/" + to_string(save_id_) + "/savedata.wg");
    if (fs::exists(save_path)) fs::remove(save_path);
    auto file = std::make_unique<FileWriter>(save_path.string());

    // Write the standard header, then the misc data version, and the misc data string tag.
    file->write_header();
    file->write_data<unsigned int>(MISC_DATA_SAVE_VERSION);
    file->write_string("MISC_DATA");

    // The only misc data to write for now is the player's region ID.
    file->write_data<int>(player_ptr_->region());

    // And the time/weather data, which is saved elsewhere.
    world_ptr_->time_weather().save_data(file.get());

    // And the EOF footer, of course.
    file->write_footer();
}

// Returns the currently-used saved game slot.
int Game::save_slot() const { return save_id_; }

// Sets the Player pointer. Use with caution.
void Game::set_player(Player* player_ptr)
{
    if (!player_ptr) throw runtime_error("Attempt to set null player pointer!");
    player_ptr_ = player_ptr;
}

// Every game needs a title screen!
void Game::title_screen()
{
    print("\n{c}Welcome to {C}Westgate {c}version " + version::VERSION_STRING + " (build " + version::BUILD_TIMESTAMP + ")");
    print("{c}Copyright (c) 2015 Raine \"Gravecat\" Simmons\n");

    print("Please select one of the following options:");
    print("{K}[{G}1{K}] {w}Start a new game");
    print("{K}[{G}2{K}] {w}Load a saved game");
    print("{K}[{G}3{K}] {w}Quit the game");

    // Right now, we're hard-coding save slot 0. Later, we'll let the user pick a save slot.
    save_id_ = 0;

    switch(terminal::get_number(1, 3))
    {
        case 1: new_game(0, "THE_CROWN_AND_SKULL"); break;
        case 2: load_game(0); break;
        case 3:
            print("{B}Farewell!");
            core().destroy_core(EXIT_SUCCESS);
            break;
    }
}

// A shortcut instead of using core().game()
Game& game() { return core().game(); }

}   // namespace westgate
