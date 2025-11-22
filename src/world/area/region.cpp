// world/area/region.cpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>

#include "core/core.hpp"
#include "util/file/binpath.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "util/file/yaml.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/region.hpp"

using std::make_unique;
using std::runtime_error;
using std::stoul;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

// Creates an empty Region.
Region::Region() : id_(0), memory_allocated_(false), name_("Undefined Region") { }

// As above, but also calls set_size() to allocate memory.
Region::Region(size_t new_size) : Region() { set_size(new_size); }

// Destructor, cleans up stored data.
Region::~Region()
{
    room_ids_.clear();
    rooms_.clear();
}

// Adds a new Room to this Region. Must be called with std::move.
void Region::add_room(unique_ptr<Room> new_room)
{
    if (!new_room) core().nonfatal("Attempted to add null room to region.", Core::CORE_ERROR);
    else rooms_.push_back(std::move(new_room));
}

// Attempts to find a room by its string ID.
Room* Region::find_room(const string& id)
{ return find_room(hash::murmur3(id)); }

// Attempts to find a room by its hashed ID.
Room* Region::find_room(uint32_t id)
{
    auto result = room_ids_.find(id);
    if (result == room_ids_.end())
    {
        core().nonfatal("Failed attempt to look up room (ID " + to_string(id) + ")", Core::CORE_ERROR);
        return nullptr;
    }
    return result->second;
}

// Retrieves this Region's unique ID.
uint32_t Region::id() const { return id_; }

// Loads this Region from a saved game file.
void Region::load_from_save(int save_slot, uint32_t region_id)
{
    // Assemble the path, and ensure the file exists.
    const fs::path save_path = BinPath::game_path("userdata/saves/" + to_string(save_slot) + "/region/" + to_string(region_id) + ".dat");
    if (!fs::exists(save_path))
        throw runtime_error("Cannot load region " + to_string(region_id) + " from save slot " + to_string(save_slot) + "!");

    // Create a FileReader to read the data.
    auto file = make_unique<FileReader>(save_path.string());

    // Check the header, save version, and region tag.
    if (!file->check_header()) throw runtime_error("Invalid region file header!");
    const uint32_t region_version = file->read_data<uint32_t>();
    if (region_version != REGION_SAVE_VERSION)
        throw runtime_error("Invalid region save version (" + to_string(region_version) + ", expected " + to_string(REGION_SAVE_VERSION) + ")");
    const string region_string = file->read_string();
    if (region_string.compare("REGION")) throw runtime_error("Invalid region file!");

    // Get the ID, name and size of the Region.
    id_ = file->read_data<uint32_t>();
    name_ = file->read_string();
    const size_t region_size = file->read_data<size_t>();
    set_size(region_size);

    // Create and load the Rooms in this Region.
    for (size_t i = 0; i < region_size; i++)
        rooms_.push_back(make_unique<Room>(file.get()));
    rebuild_room_id_map();

    // Check for the standard EOF footer.
    if (!file->check_footer()) throw runtime_error("Invalid region file footer!");
}

// Loads a Region from YAML game data.
void Region::load_from_gamedata(const string& filename)
{
    // Determine this region's ID from the filename.
    auto dash_pos = filename.find_first_of('-');
    if (dash_pos == string::npos) throw runtime_error("Cannot determine region ID: " + filename);
    try { id_ = stoul(filename.substr(0, dash_pos)); }
    catch (std::invalid_argument&) { throw runtime_error("Invalid region ID: " + filename); }

    // Determine the full path for the data file, and ensure the file exists.
    const string full_filename = core().datafile("world/regions/" + filename);
    if (!fs::exists(full_filename)) throw runtime_error("Could not locate region file: " + filename);

    // Load the YAML file into memory.
    const YAML yaml(full_filename);
    if (!yaml.is_map()) throw runtime_error(filename + ": Invalid file format!");

    // Get the region identifier data.
    const YAML region_id = yaml.get_child("REGION_IDENTIFIER");
    if (!region_id.is_map()) throw runtime_error(filename + ": Cannot find region identifier data!");
    if (!region_id.key_exists("version")) throw runtime_error(filename + ": Missing version in identifier data!");
    uint32_t region_version;
    try { region_version = stoul(region_id.val("version")); }
    catch (std::invalid_argument&) { throw runtime_error(filename + ": Invalid region version identifier!"); }
    if (region_version != REGION_YAML_VERSION) throw runtime_error(filename + ": Invalid region version (" + to_string(region_version) +
        ", expected " + to_string(REGION_YAML_VERSION) + ")");
    if (!region_id.key_exists("name")) throw runtime_error(filename + ": Missing region name in identifier data!");
    name_ = region_id.val("name");

    // Get all the keys in this region, and iterate over them one at a time.
    const vector<string> region_keys = yaml.keys();
    set_size(region_keys.size() - 1);
    for (auto key : region_keys)
    {
        if (key == "REGION_IDENTIFIER") continue;   // Skip the region identifier section, we did that already.

        const YAML room_yaml = yaml.get_child(key);
        const string error_str = filename + " [" + key + "]: ";
        unique_ptr<Room> room_ptr = make_unique<Room>(key);

        if (!room_yaml.key_exists("name")) throw runtime_error(error_str + "Missing name data.");
        if (!room_yaml.get_child("name").is_seq()) throw runtime_error(error_str + "Name data not correctly set (expected sequence).");
        const vector<string> name_vec = room_yaml.get_seq("name");
        if (name_vec.size() != 2) throw runtime_error(error_str + "Name data not correctly set (expected sequence of length 2, got " +
            to_string(name_vec.size()) + ".");
        room_ptr->set_name(name_vec.at(0), name_vec.at(1));

        if (!room_yaml.key_exists("desc")) throw runtime_error(error_str + "Missing room description.");
        room_ptr->set_desc(stringutils::strip_trailing_newlines(room_yaml.val("desc")));

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
    const fs::path region_saves_path = BinPath::game_path("userdata/saves/" + to_string(save_slot) + "/region");
    if (!fs::exists(region_saves_path)) fs::create_directory(region_saves_path);

    // Delete any old data if it's currently there.
    const fs::path region_save_file = region_saves_path.string() + "/" + to_string(id_) + ".dat";
    if (fs::exists(region_save_file)) fs::remove(region_save_file);

    // Create the save file, and mark it with a version tag.
    auto file = make_unique<FileWriter>(region_save_file.string());
    file->write_header();
    file->write_data<uint32_t>(REGION_SAVE_VERSION);
    file->write_string("REGION");

    // Write the ID, name, and size of the region.
    file->write_data<uint32_t>(id_);
    file->write_string(name_);
    file->write_data<size_t>(rooms_.size());

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
