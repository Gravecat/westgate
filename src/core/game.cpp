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
#include "world/codex.hpp"
#include "world/entity/player.hpp"

using std::make_unique;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Constructor, sets up the game manager.
Game::Game() : codex_ptr_(nullptr), player_ptr_(nullptr), region_ptr_(nullptr), save_id_(-1) { }

// Destructor, cleans up attached classes.
Game::~Game()
{
    codex_ptr_.reset(nullptr);
    region_ptr_.reset(nullptr);
}

// Starts the game, in the form of a title screen followed by the main game loop.
void Game::begin()
{
    codex_ptr_ = make_unique<Codex>();
    title_screen();
    print();
    player_ptr_->parent_room()->look();
    main_loop();
}

// Returns a reference to the Codex object.
Codex& Game::codex() const
{
    if (!codex_ptr_) throw runtime_error("Attempt to access null Codex pointer!");
    return *codex_ptr_;
}

// Loads the static YAML data and generates a binary save file for the game world.
void Game::create_world()
{
    // This can be replaced with something better later.
    print("{c}Generating game world from static data...");

    // Create a game saves folder, if one doesn't already exist.
    const fs::path userdata_saves_path = BinPath::game_path("userdata/saves");
    if (!fs::is_directory(userdata_saves_path)) fs::create_directory(userdata_saves_path);

    // Right now, we're hard-coding save slot 0. Later in development, we'll let the user choose multiple save slots.
    save_id_ = 0;

    // This early in development, we're gonna just delete the save folder each time. It'll become more permanent later.
    const fs::path save_dir = BinPath::merge_paths(userdata_saves_path.string(), to_string(save_id_));
    fs::remove_all(save_dir);
    fs::create_directory(save_dir);

    // Determine how many region files are in the game's data files.
    const fs::path regions_folder = core().datafile("world/regions");
    vector<fs::path> regions;
    for (const auto& file : fs::directory_iterator(regions_folder))
        if (file.is_regular_file()) regions.push_back(file.path().filename());

    // One at a time, load each region into memory.
    for (size_t i = 0; i < regions.size(); i++)
    {
        print("{c}Processing region file {C}" + to_string(i + 1) + " {c}of {C}" + to_string(regions.size()) + "{c}...");
        fs::path region_file = regions.at(i);
        unique_ptr<Region> new_region = make_unique<Region>();
        new_region->load_from_gamedata(region_file.string());
        new_region->save(save_id_);
    }
    print("{c}World generation complete!");
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
    const fs::path meta_path = BinPath::merge_paths(save_path.string(), "meta.dat");
    if (!fs::exists(meta_path)) throw runtime_error("Could not locate saved game metadata!");
    auto file = make_unique<FileReader>(meta_path.string());

    // Check the metadata headers and version.
    if (!file->check_header()) throw runtime_error("Invalid metadata header!");
    const uint32_t meta_version = file->read_data<uint32_t>();
    if (meta_version != METADATA_SAVE_VERSION) throw runtime_error("Invalid metadata version (" + to_string(meta_version) + ", expected " +
        to_string(METADATA_SAVE_VERSION) + ")");
    if (file->read_string() != "METADATA") throw runtime_error("Invalid metadata header!");

    // Check the currently-loaded Region.
    const uint32_t current_region = file->read_data<uint32_t>();

    // Finally, check the footer before closing the file.
    if (!file->check_footer()) throw runtime_error("Invalid metadata footer!");
    file.reset(nullptr);

    auto new_region = make_unique<Region>();
    new_region->load_from_save(save_slot, current_region);
    if (!player_ptr_) throw runtime_error("Could not locate player character in saved region!");
    region_ptr_ = std::move(new_region);

    print("{Y}Saved game loaded successfully!");
}

// brøether, may i have the lööps
void Game::main_loop() { while(true) { parser::process_input(terminal::get_input()); } }

// Sets up for a new game!
void Game::new_game(const uint32_t starting_region, const string& starting_room)
{
    create_world(); // In the beginning, there was darkness.

    // Create a new Region in memory, then load it from disk.
    region_ptr_ = make_unique<Region>();
    region_ptr_->load_from_save(save_id_, starting_region);

    // Create the player character, assign them to a starting room, then transfer ownership.
    auto player = make_unique<Player>(nullptr);
    Room* start_room = region_ptr_->find_room(starting_room);
    start_room->add_entity(std::move(player));

    // Save the game silently, to store the player character.
    save(false);
}

// Returns a reference to the Player object.
Player& Game::player() const { return *player_ptr_; }

// Save the game, if there's a game in progress.
void Game::save(bool chatty)
{
    if (!region_ptr_)
    {
        if (chatty) print("{R}There is no game in progress!");
        return;
    }
    if (chatty) print("{B}Saving the game...", false);
    save_metadata();
    region_ptr_->save(save_id_);
    if (chatty) print(" Done!");
}

// Saves a metadata save file, which contains basic info like the current region and save file version.
void Game::save_metadata()
{
    const fs::path save_path = BinPath::game_path("userdata/saves/" + to_string(save_id_) + "/meta.dat");
    if (fs::exists(save_path)) fs::remove(save_path);
    auto file = make_unique<FileWriter>(save_path.string());

    // Write the standard header, then the metadata version, and the metadata string tag.
    file->write_header();
    file->write_data<uint32_t>(METADATA_SAVE_VERSION);
    file->write_string("METADATA");

    // The only other thing to write for now is the region's ID.
    file->write_data<uint32_t>(region_ptr_->id());

    // And the EOF footer, of course.
    file->write_footer();
}

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
