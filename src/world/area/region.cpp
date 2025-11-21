// world/area/region.cpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>
#include <stdexcept>

#include "core/core.hpp"
#include "util/file/binpath.hpp"
#include "util/file/filewriter.hpp"
#include "util/file/yaml.hpp"
#include "world/area/region.hpp"

namespace westgate {

// Creates an empty Region.
Region::Region() : id_(0), memory_allocated_(false), name_("Undefined Region") { }

// As above, but also calls set_size() to allocate memory.
Region::Region(size_t new_size) : memory_allocated_(false), name_("Undefined Region") { set_size(new_size); }

// Destructor, cleans up stored data.
Region::~Region()
{
    room_ids_.clear();
    rooms_.clear();
}

// Adds a new Room to this Region. Must be called with std::move.
void Region::add_room(std::unique_ptr<Room> new_room)
{
    if (!new_room) core().nonfatal("Attempted to add null room to region.", Core::CORE_ERROR);
    else rooms_.push_back(std::move(new_room));
}

// Loads a Region from YAML game data.
void Region::load_from_gamedata(const std::string& filename)
{
    // Determine this region's ID from the filename.
    auto dash_pos = filename.find_first_of('-');
    if (dash_pos == std::string::npos) throw std::runtime_error("Cannot determine region ID: " + filename);
    try { id_ = std::stoul(filename.substr(0, dash_pos)); }
    catch (std::invalid_argument&) { throw std::runtime_error("Invalid region ID: " + filename); }

    // Determine the full path for the data file, and ensure the file exists.
    const std::string full_filename = core().datafile("world/regions/" + filename);
    if (!std::filesystem::exists(full_filename)) throw std::runtime_error("Could not locate region file: " + filename);

    // Load the YAML file into memory.
    const YAML yaml(full_filename);
    if (!yaml.is_map()) throw std::runtime_error(filename + ": Invalid file format!");

    // Get the region identifier data.
    const YAML region_id = yaml.get_child("REGION_IDENTIFIER");
    if (!region_id.is_map()) throw std::runtime_error(filename + ": Cannot find region identifier data!");
    if (!region_id.key_exists("version")) throw std::runtime_error(filename + ": Missing version in identifier data!");
    unsigned int region_version;
    try { region_version = std::stoul(region_id.val("version")); }
    catch (std::invalid_argument&) { throw std::runtime_error(filename + ": Invalid region version identifier!"); }
    if (region_version != REGION_YAML_VERSION) throw std::runtime_error(filename + ": Invalid region version (" + std::to_string(region_version) +
        ", expected " + std::to_string(REGION_YAML_VERSION) + ")");
    if (!region_id.key_exists("name")) throw std::runtime_error(filename + ": Missing region name in identifier data!");
    name_ = region_id.val("name");

    // Get all the keys in this region, and iterate over them one at a time.
    const std::vector<std::string> region_keys = yaml.keys();
    set_size(region_keys.size() - 1);
    for (auto key : region_keys)
    {
        if (key == "REGION_IDENTIFIER") continue;   // Skip the region identifier section, we did that already.

        const YAML room_yaml = yaml.get_child(key);
        const std::string error_str = filename + " [" + key + "]: ";
        std::unique_ptr<Room> room_ptr = std::make_unique<Room>(key);

        if (!room_yaml.key_exists("name")) throw std::runtime_error(error_str + "Missing name data.");
        if (!room_yaml.get_child("name").is_seq()) throw std::runtime_error(error_str + "Name data not correctly set (expected sequence).");
        const std::vector<std::string> name_vec = room_yaml.get_seq("name");
        if (name_vec.size() != 2) throw std::runtime_error(error_str + "Name data not correctly set (expected sequence of length 2, got " +
            std::to_string(name_vec.size()) + ".");
        room_ptr->set_name(name_vec.at(0), name_vec.at(1));

        if (!room_yaml.key_exists("desc")) throw std::runtime_error(error_str + "Missing room description.");
        room_ptr->set_desc(room_yaml.val("desc"));

        // Add the Room to the Region.
        rooms_.push_back(std::move(room_ptr));
    }

    // Rebuilds the room ID map, for faster lookup of room IDs.
    rebuild_room_id_map();
}

// Rebuilds the room ID map, for quickly looking up Rooms by their hashed ID.
void Region::rebuild_room_id_map()
{
    if (room_ids_.size())
    {
        core().nonfatal("Rebuilding room ID map when one already exists.", Core::CORE_WARN);
        room_ids_.clear();
    }
    for (auto &room : rooms_)
        room_ids_.insert({room->id(), room.get()});
}

// Saves the Region to a saved game file.
void Region::save(int save_slot)
{
    // Ensure the correct folder exists.
    const std::filesystem::path region_saves_path = BinPath::game_path("userdata/saves/" + std::to_string(save_slot) + "/region");
    if (!std::filesystem::exists(region_saves_path)) std::filesystem::create_directory(region_saves_path);

    // Delete any old data if it's currently there.
    const std::filesystem::path region_save_file = region_saves_path.string() + "/" + std::to_string(id_) + ".dat";
    if (std::filesystem::exists(region_save_file)) std::filesystem::remove(region_save_file);

    // Create the save file, and mark it with a version tag.
    auto file = std::make_unique<FileWriter>(region_save_file.string());
    file->write_header();
    file->write_data<int>(REGION_SAVE_VERSION);
    file->write_string("REGION");

    // Write the ID, name, and size of the region.
    file->write_data<unsigned int>(id_);
    file->write_string(name_);
    file->write_data<unsigned int>(rooms_.size());

    // Instruct each contained Room to save itself.
    for (auto &room : rooms_)
        room->save(file.get());

    // Write an EOF tag, so we know the end is where it should be.
    file->write_footer();
}

// Reallocates memory for the rooms_ vector, if we know exactly how large it's gonna be.
void Region::set_size(size_t new_size)
{
    if (memory_allocated_) core().nonfatal("Attempting to resize a Region that already has a defined size.", Core::CORE_ERROR);
    rooms_.reserve(new_size);
    memory_allocated_ = true;
}

}   // namespace westgate
