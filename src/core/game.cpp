// core/game.cpp -- The Game object is the central game manager, handling the main loop, saving/loading, and starting of new games.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <cstdlib>
#include <filesystem>
#include <stdexcept>

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
    codex_ptr_ = std::make_unique<Codex>();
    title_screen();
    terminal::print();
    player_ptr_->parent_room()->look();
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
    terminal::print("{c}Generating game world from static data...");

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
        terminal::print("{c}Processing region file {C}" + std::to_string(i + 1) + " {c}of {C}" + std::to_string(regions.size()) + "{c}...");
        std::filesystem::path region_file = regions.at(i);
        std::unique_ptr<Region> new_region = std::make_unique<Region>();
        new_region->load_from_gamedata(region_file.string());
        new_region->save(save_id_);
    }
    terminal::print("{c}World generation complete!");
}

// Shuts things down cleanly and exits the game.
void Game::leave_game() { core().destroy_core(EXIT_SUCCESS); }

// Loads an existing saved game.
void Game::load_game(int save_slot)
{
    player_ptr_ = nullptr;
    const std::filesystem::path save_path = BinPath::game_path("userdata/saves/" + std::to_string(save_slot));
    if (!std::filesystem::exists(save_path))
    {
        terminal::print("{R}Saved game file cannot be located.");
        core().destroy_core(EXIT_SUCCESS);
    }

    // Load the metadata file.
    const std::filesystem::path meta_path = BinPath::merge_paths(save_path.string(), "meta.dat");
    if (!std::filesystem::exists(meta_path)) throw std::runtime_error("Could not locate saved game metadata!");
    auto file = std::make_unique<FileReader>(meta_path.string());

    // Check the metadata headers and version.
    if (!file->check_header()) throw std::runtime_error("Invalid metadata header!");
    const uint32_t meta_version = file->read_data<uint32_t>();
    if (meta_version != METADATA_SAVE_VERSION) throw std::runtime_error("Invalid metadata version (" + std::to_string(meta_version) + ", expected " +
        std::to_string(METADATA_SAVE_VERSION) + ")");
    if (file->read_string() != "METADATA") throw std::runtime_error("Invalid metadata header!");

    // Check the currently-loaded Region.
    const uint32_t current_region = file->read_data<uint32_t>();

    // Finally, check the footer before closing the file.
    if (!file->check_footer()) throw std::runtime_error("Invalid metadata footer!");
    file.reset(nullptr);

    auto new_region = std::make_unique<Region>();
    new_region->load_from_save(save_slot, current_region);
    if (!player_ptr_) throw std::runtime_error("Could not locate player character in saved region!");
    region_ptr_ = std::move(new_region);

    terminal::print("{Y}Saved game loaded successfully!");
}

// brøether, may i have the lööps
void Game::main_loop() { while(true) { parser::process_input(terminal::get_input()); } }

// Sets up for a new game!
void Game::new_game()
{
    create_world(); // In the beginning, there was darkness.

    // Create a new Region in memory, then load it from disk.
    region_ptr_ = std::make_unique<Region>();
    region_ptr_->load_from_save(save_id_, 0);

    // Create the player character, assign them to a starting room, then transfer ownership.
    auto player = std::make_unique<Player>(nullptr);
    Room* start_room = region_ptr_->find_room("SULA_PLAINS");
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
        if (chatty) terminal::print("{R}There is no game in progress!");
        return;
    }
    if (chatty) terminal::print("{B}Saving the game...", false);
    save_metadata();
    region_ptr_->save(save_id_);
    if (chatty) terminal::print(" Done!");
}

// Saves a metadata save file, which contains basic info like the current region and save file version.
void Game::save_metadata()
{
    const std::filesystem::path save_path = BinPath::game_path("userdata/saves/" + std::to_string(save_id_) + "/meta.dat");
    if (std::filesystem::exists(save_path)) std::filesystem::remove(save_path);
    auto file = std::make_unique<FileWriter>(save_path.string());

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
    if (!player_ptr) throw std::runtime_error("Attempt to set null player pointer!");
    player_ptr_ = player_ptr;
}

// Every game needs a title screen!
void Game::title_screen()
{
    terminal::print("\n{c}Welcome to {C}Westgate {c}version " + version::VERSION_STRING + " (build " + version::BUILD_TIMESTAMP + ")");
    terminal::print("{c}Copyright (c) 2015 Raine \"Gravecat\" Simmons\n");

    terminal::print("Please select one of the following options:");
    terminal::print("{K}[{G}1{K}] {w}Start a new game");
    terminal::print("{K}[{G}2{K}] {w}Load a saved game");
    terminal::print("{K}[{G}3{K}] {w}Quit the game");

    switch(terminal::get_number(1, 3))
    {
        case 1: new_game(); break;
        case 2: load_game(0); break;
        case 3:
            terminal::print("{B}Farewell!");
            core().destroy_core(EXIT_SUCCESS);
            break;
    }
}

// A shortcut instead of using core().game()
Game& game() { return core().game(); }

}   // namespace westgate
