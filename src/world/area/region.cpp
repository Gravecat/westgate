// world/area/region.cpp -- A Region is a class managing a collection of Rooms. This will allow for loading and unloading of sections of the game world at a
// time, without having to keep everything in memory at once.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <filesystem>

#include "core/core.hpp"
#include "core/game.hpp"
#include "parser/parser.hpp"
#include "util/file/binpath.hpp"
#include "util/file/filereader.hpp"
#include "util/file/filewriter.hpp"
#include "util/file/yaml.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/automap.hpp"
#include "world/area/region.hpp"
#include "world/world.hpp"

using std::invalid_argument;
using std::make_unique;
using std::out_of_range;
using std::runtime_error;
using std::stol;
using std::stoul;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

// Creates an empty Region.
Region::Region() : id_(0), name_("Undefined Region") { }

// Destructor, cleans up stored data.
Region::~Region()
{ rooms_.clear(); }

// Attempts to find a room by its string ID.
Room* Region::find_room(const string& id) const
{ return find_room(hash::murmur3(id)); }

// Attempts to find a room by its hashed ID.
Room* Region::find_room(uint32_t id) const
{
    auto result = rooms_.find(id);
    if (result == rooms_.end())
    {
        core().nonfatal("Failed attempt to look up room (ID " + to_string(id) + ")", Core::CORE_ERROR);
        return nullptr;
    }
    return result->second.get();
}

// Retrieves this Region's unique ID.
uint32_t Region::id() const { return id_; }

// Loads this Region's YAML data, then applies delta changes from saved game binary data.
void Region::load(int save_slot, uint32_t region_id)
{
    // Check all the YAML files in the regions data folder.
    const fs::path regions_folder = core().datafile("world/regions");
    string yaml_filename;
    for (const auto& file : fs::directory_iterator(regions_folder))
    {
        if (file.is_regular_file())
        {
            // Determine this region's ID from the filename.
            const string filename = file.path().filename().string();
            auto dash_pos = filename.find_first_of('-');
            if (dash_pos == string::npos) throw runtime_error("Cannot determine region ID: " + filename);
            try { id_ = stoul(filename.substr(0, dash_pos)); }
            catch (invalid_argument&) { throw runtime_error("Invalid region ID: " + filename); }
            if (id_ == region_id)
            {
                yaml_filename = filename;
                break;
            }
        }
    }
    if (!yaml_filename.size()) throw runtime_error("Unable to locate data for region ID: " + to_string(region_id));

    // Load the YAML data, then apply delta changes on top of that from the save file.
    load_from_gamedata(yaml_filename, true);
    load_delta(save_slot);
}

// Loads delta changes from a saved game file.
void Region::load_delta(int save_slot)
{
    // Ensure the save file exists.
    const string err_file = " (slot " + to_string(save_slot) + ", region " + to_string(id_) + ")";
    const string save_file = BinPath::game_path("userdata/saves/" + to_string(save_slot) + "/region/" + to_string(id_) + ".wg");
    if (!fs::is_regular_file(save_file)) throw runtime_error("Unable to load region deltas" + err_file);

    // Load the save file, check the headers and version.
    auto file = make_unique<FileReader>(save_file);
    if (!file->check_header()) throw runtime_error("Invalid region deltas" + err_file);
    const uint32_t delta_ver = file->read_data<uint32_t>();
    if (delta_ver != REGION_SAVE_VERSION) FileReader::standard_error("Invalid region deltas save version" + err_file, delta_ver, REGION_SAVE_VERSION);
    if (file->read_string().compare("REGION_DELTA")) throw runtime_error("Invalid region deltas" + err_file);
    const uint32_t delta_id = file->read_data<uint32_t>();
    if (delta_id != id_) FileReader::standard_error("Mismatched region delta ID" + err_file, delta_id, id_);

    // Load the Room deltas, if any.
    while(true)
    {
        const uint32_t delta_tag = file->read_data<uint32_t>();
        if (delta_tag == REGION_DELTA_ROOMS_END) break;
        else if (delta_tag == REGION_DELTA_ROOM)
        {
            const uint32_t room_ver = file->read_data<uint32_t>();
            if (room_ver != Room::ROOM_SAVE_VERSION) FileReader::standard_error("Invalid region room version", room_ver, Room::ROOM_SAVE_VERSION);
            const uint32_t room_id = file->read_data<uint32_t>();
            auto result = rooms_.find(room_id);
            if (result == rooms_.end()) throw std::runtime_error("Could not locate room " + to_string(room_id) + " in region " + to_string(id_));
            result->second->load_delta(file.get());
        }
        else throw runtime_error("Unknown region delta tag: " + to_string(delta_tag));
    }

    // Check for the correct EOF footer.
    if (!file->check_footer()) throw runtime_error("Invalid region deltas" + err_file);
}

// Loads a Region from YAML game data.
void Region::load_from_gamedata(const string& filename, bool update_world)
{
    // Determine this region's ID from the filename.
    auto dash_pos = filename.find_first_of('-');
    if (dash_pos == string::npos) throw runtime_error("Cannot determine region ID: " + filename);
    try { id_ = stoul(filename.substr(0, dash_pos)); }
    catch (invalid_argument&) { throw runtime_error("Invalid region ID: " + filename); }

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
    catch (invalid_argument&) { throw runtime_error(filename + ": Invalid region version identifier!"); }
    if (region_version != REGION_YAML_VERSION) FileReader::standard_error("Invalid region version", region_version, REGION_YAML_VERSION, {filename});
    if (!region_id.key_exists("name")) throw runtime_error(filename + ": Missing region name in identifier data!");
    name_ = region_id.val("name");

    // Get all the keys in this region, and iterate over them one at a time.
    const vector<string> region_keys = yaml.keys();
    for (auto key : region_keys)
    {
        if (key == "REGION_IDENTIFIER") continue;   // Skip the region identifier section, we did that already.

        const YAML room_yaml = yaml.get_child(key);
        const string error_str = filename + " [" + key + "]: ";
        unique_ptr<Room> room_ptr = make_unique<Room>(key);

        if (!room_yaml.key_exists("short_name")) throw runtime_error(error_str + "Missing short_name data.");
        room_ptr->set_short_name(room_yaml.val("short_name"), false);

        if (!room_yaml.key_exists("desc")) throw runtime_error(error_str + "Missing room description.");
        room_ptr->set_desc(stringutils::strip_trailing_newlines(room_yaml.val("desc")), false);

        if (!room_yaml.key_exists("coords")) throw runtime_error(error_str + "Missing room coordinates.");
        if (!room_yaml.get_child("coords").is_seq()) throw runtime_error(error_str + "Coordinate data not correctly set (expected sequence).");
        const vector<string> coord_vec = room_yaml.get_seq("coords");
        if (coord_vec.size() != 3) FileReader::standard_error("Coord data sequence length incorrect", 3, coord_vec.size(), {key});
        vector<int32_t> coord_int_vec(3);
        for (int i = 0; i < 3; i++)
        {
            try
            { coord_int_vec.at(i) = stol(coord_vec.at(i));}
            catch(const invalid_argument&)
            { throw runtime_error(error_str + "Coordinate data could not be processed (not a number?)"); }
            catch(const out_of_range&)
            { throw runtime_error(error_str + "Coordinate data could not be processed (out of range?)"); }
        }
        room_ptr->set_coords(Vector3(coord_int_vec.at(0), coord_int_vec.at(1), coord_int_vec.at(2)));

        if (!room_yaml.key_exists("map")) throw runtime_error(error_str + "Missing map character.");
        room_ptr->set_map_char(room_yaml.val("map"), false);

        // If the Room has any exits, process them here.
        if (room_yaml.key_exists("exits"))
        {
            auto room_exits = room_yaml.get_child("exits").keys_vals();
            for (auto &exit : room_exits)
            {
                Direction dir = parser::parse_direction(hash::murmur3(exit.first));
                if (dir == Direction::NONE) throw runtime_error(error_str + "Invalid room exits.");
                room_ptr->set_exit(dir, hash::murmur3(exit.second), false);
            }
        }

        // If requested, update the lookup tables for Rooms and Regions.
        if (update_world)
        {
            world().add_room_to_region(room_ptr->id(), id_);
            world().automap().add_room_vec(room_ptr->id(), room_ptr->coords());
        }

        // Add the Room to the Region.
        rooms_.insert({room_ptr->id(), std::move(room_ptr)});
    }
}

// Saves only the changes to this Region in a save file.
void Region::save_delta(int save_slot, bool no_changes)
{
    // Ensure the correct folder exists.
    const fs::path region_saves_path = BinPath::game_path("userdata/saves/" + to_string(save_slot) + "/region");
    if (!fs::exists(region_saves_path)) fs::create_directory(region_saves_path);

    // Delete any old data if it's currently there.
    const fs::path region_save_file = BinPath::merge_paths(region_saves_path.string(), to_string(id_) + ".wg");
    if (fs::exists(region_save_file)) fs::remove(region_save_file);

    // Create the save file, and mark it with a version tag.
    auto file = make_unique<FileWriter>(region_save_file.string());
    file->write_header();
    file->write_data<uint32_t>(REGION_SAVE_VERSION);
    file->write_string("REGION_DELTA");
    file->write_data<uint32_t>(id_);

    if (!no_changes)
    {
        // Instruct each contained Room to save its deltas, if any.
        for (auto &room : rooms_)
            room.second->save_delta(file.get());
    }
    file->write_data<uint32_t>(REGION_DELTA_ROOMS_END);

    // Write an EOF tag, so we know the end is where it should be.
    file->write_footer();
}

}   // namespace westgate
