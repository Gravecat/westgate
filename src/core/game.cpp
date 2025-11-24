// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>

#include "cmake/version.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "util/file/binpath.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "world/area/region.hpp"
#include "world/entity/player.hpp"
#include "world/world.hpp"

using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Constructor, sets up the game manager.
Game::Game() : player_ptr_(nullptr), save_id_(-1), world_ptr_(nullptr) { }

// Destructor, cleans up attached classes.
Game::~Game()
{ world_ptr_.reset(nullptr); }

// Starts the game, in the form of a title screen followed by the main game loop.
void Game::begin()
{
    world_ptr_ = make_unique<World>();
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
    player_ptr_ = nullptr;
    const fs::path save_path = BinPath::game_path("userdata/saves/" + to_string(save_slot));
    if (!fs::exists(save_path))
    {
        print("{R}Saved game file cannot be located.");
        core().destroy_core(EXIT_SUCCESS);
    }

    // Load the metadata file.
    const fs::path meta_path = BinPath::merge_paths(save_path.string(), "meta.wg");
    if (!fs::exists(meta_path)) throw runtime_error("Could not locate saved game metadata!");
    auto file = make_unique<FileReader>(meta_path.string());

    // Check the metadata headers and version.
    if (!file->check_header()) throw runtime_error("Invalid metadata header!");
    const uint32_t meta_version = file->read_data<uint32_t>();
    if (meta_version != METADATA_SAVE_VERSION) FileReader::standard_error("Invalid metadata version", meta_version, METADATA_SAVE_VERSION);
    if (file->read_string() != "METADATA") throw runtime_error("Invalid metadata header!");

    // Check what Region the player is in.
    const uint32_t current_region = file->read_data<uint32_t>();

    // Finally, check the footer before closing the file.
    if (!file->check_footer()) throw runtime_error("Invalid metadata footer!");
    file.reset(nullptr);

    // Load the Region that contains the Player object.
    world_ptr_->load_region(current_region);

    print("{Y}Saved game loaded successfully!");
}

// brøether, may i have the lööps
void Game::main_loop() { while(true) { parser::process_input(terminal::get_input()); } }

// Sets up for a new game!
void Game::new_game(const uint32_t starting_region, const string& starting_room)
{
    // Create the new region delta save files.
    world_ptr_->create_region_saves(save_id_);

    // Create the player character, assign them to a starting room, then transfer ownership.
    auto player = make_unique<Player>(nullptr);
    Room* start_room = world_ptr_->find_room(starting_room, starting_region);
    start_room->add_entity(std::move(player));

    // Save the game silently, to store the player character.
    save(false);
}

// Returns a reference to the Player object.
Player& Game::player() const { return *player_ptr_; }

// Save the game, if there's a game in progress.
void Game::save(bool chatty)
{
    if (chatty) print("{B}Saving the game...", false);
    world_ptr_->save(save_id_);
    save_metadata();
    if (chatty) print(" Done!");
}

// Saves a metadata save file, which contains basic info like the current region and save file version.
void Game::save_metadata()
{
    const fs::path save_path = BinPath::game_path("userdata/saves/" + to_string(save_id_) + "/meta.wg");
    if (fs::exists(save_path)) fs::remove(save_path);
    auto file = make_unique<FileWriter>(save_path.string());

    // Write the standard header, then the metadata version, and the metadata string tag.
    file->write_header();
    file->write_data<uint32_t>(METADATA_SAVE_VERSION);
    file->write_string("METADATA");

    // The only other thing to write for now is the player's region ID.
    file->write_data<uint32_t>(player_ptr_->region());

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
        case 1: new_game(0, "SULA_PLAINS"); break;
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
