// world/area/room.cpp -- The Room class defines a singular place in the game world, connected via links to other Rooms. Entities all exist within Rooms.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/filex.hpp"
#include "util/strx.hpp"
#include "world/area/automap.hpp"
#include "world/area/region.hpp"
#include "world/area/room.hpp"
#include "world/entity/mobile.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"
#include "world/world.hpp"

using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
using westgate::terminal::print;

namespace westgate {

// Lookup table to convert a Direction enum into a string name.
const string Room::direction_names_[11] = { "", "north", "northeast", "east", "southeast", "south", "southwest", "west", "northwest", "up", "down" };

// Lookup table that inverts a Direction (e.g. east -> west).
const Direction Room::reverse_direction_map_[11] = { Direction::NONE, Direction:: SOUTH, Direction::SOUTHWEST, Direction::WEST, Direction::NORTHWEST,
    Direction::NORTH, Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST, Direction::DOWN, Direction::UP };

// Used during loading YAML data, to convert RoomTag text names into RoomTag enums.
const std::map<std::string, RoomTag> Room::tag_map_ = { {"Explored", RoomTag::Explored }, { "Indoors", RoomTag::Indoors }, { "Windows", RoomTag::Windows },
    { "City", RoomTag::City }, { "Underground", RoomTag::Underground }, { "Trees", RoomTag::Trees }, { "AlwaysWinter", RoomTag::AlwaysWinter },
    { "AlwaysSpring", RoomTag::AlwaysSpring }, { "AlwaysSummer", RoomTag::AlwaysSummer }, { "AlwaysAutumn", RoomTag::AlwaysAutumn },
    { "UnfinishedNorth", RoomTag::UnfinishedNorth }, { "UnfinishedNortheast", RoomTag::UnfinishedNortheast }, { "UnfinishedEast", RoomTag::UnfinishedEast },
    { "UnfinishedSoutheast", RoomTag::UnfinishedSoutheast }, { "UnfinishedSouth", RoomTag::UnfinishedSouth },
    { "UnfinishedSouthwest", RoomTag::UnfinishedSouthwest }, { "UnfinishedWest", RoomTag::UnfinishedWest },
    { "UnfinishedNorthwest", RoomTag::UnfinishedNorthwest }, { "UnfinishedUp", RoomTag::UnfinishedUp }, { "UnfinishedDown", RoomTag::UnfinishedDown } };

// Lookup table for unfinished exit links.
const RoomTag Room::unfinished_directions_[10] = { RoomTag::UnfinishedNorth, RoomTag::UnfinishedNortheast, RoomTag::UnfinishedEast,
    RoomTag::UnfinishedSoutheast, RoomTag::UnfinishedSouth, RoomTag::UnfinishedSouthwest, RoomTag::UnfinishedWest, RoomTag::UnfinishedNorthwest,
    RoomTag::UnfinishedUp, RoomTag::UnfinishedDown };

// Creates a blank Room with default values and no ID.
Room::Room() : desc_("Missing room description."), links_{}, id_(0), map_char_("{M}?"), name_{"undefined", "undefined"} { }

// Creates a Room with a specified ID.
Room::Room(const string& new_id) : Room()
{
    id_str_ = new_id;
    id_ = StrX::murmur3(new_id);
}

// Adds an Entity to this room directly. Use transfer() to move Entities between rooms.
void Room::add_entity(unique_ptr<Entity> entity)
{
    entity->set_parent_room(this);
    entities_.push_back(std::move(entity));
}

// Checks if we can see the outside world from here.
bool Room::can_see_outside() const
{
    // If the Room isn't tagged as Indoors or Underground, then it's de facto outside, so we can see outside.
    if (!tag(RoomTag::Indoors) && !tag(RoomTag::Underground)) return true;

    // If the Room is tagged with Windows, this implies a view of the outside. Technically, any Room could be tagged with Windows even if it wasn't linked to
    // any outside Room, but rather than checking everything, it's better to just trust the area-builder knew what they were doing.
    if (tag(RoomTag::Windows)) return true;

    // So we probably can't see outside. Let's check if any Links are tagged as SeeThrough or Open, and if so, check if they connect to an exterior room.
    for (auto &link : links_)
    {
        if (!link) continue;    // Ignore dead links, obviously.
        if (link->tag(LinkTag::Openable))   // Doors will block our vision, unless...
        {
            // If it's not Open and not SeeThrough, then we're out of luck.
            if (!link->tag(LinkTag::Open) && !link->tag(LinkTag::SeeThrough)) continue;
        }

        // Check the linked room, to see if it's outdoors.
        Room* linked_room = world().find_room(link->get());
        if (linked_room->tag(RoomTag::Indoors) || linked_room->tag(RoomTag::Underground)) continue;
        else return true;
    }

    return false;
}

// Clears a LinkTag on a specifieid Link.
void Room::clear_link_tag(Direction dir, LinkTag tag, bool mark_delta)
{
    int array_pos = link_id(dir, "clear_link_tag", true);
    links_[array_pos]->clear_tag(tag, mark_delta);
    if (mark_delta) set_tag(RoomTag::ChangedExits);
}

// Clears multiple LinkTags at once.
void Room::clear_link_tags(Direction dir, std::list<LinkTag> tags_list, bool mark_delta)
{
    int array_pos = link_id(dir, "clear_link_tags", true);
    links_[array_pos]->clear_tags(tags_list, mark_delta);
    if (mark_delta) set_tag(RoomTag::ChangedExits);
}

// Clears a RoomTag from this Room.
void Room::clear_tag(RoomTag the_tag, bool mark_delta)
{
    if (!(tags_.count(the_tag) > 0)) return;
    tags_.erase(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Clears multiple RoomTags at the same time.
void Room::clear_tags(std::list<RoomTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        clear_tag(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Gets the string name of a Direction enum.
const string& Room::direction_name(Direction dir)
{
    if (dir > Direction::DOWN) throw runtime_error("Invalid direction call from direction_name");
    return direction_names_[static_cast<int>(dir)];
}

// Returns the name of the door (door, gate, etc.) on the specified Link, if any.
const std::string Room::door_name(Direction dir) const
{
    int array_pos = link_id(dir, "clear_link_tag", false);
    if (!links_[array_pos]) return "";
    else return links_[array_pos]->door_name();
}

// Gets the Room linked in the specified direction, or nullptr if none is linked.
Room* Room::get_link(Direction dir)
{
    const uint8_t array_pos = static_cast<uint8_t>(dir) - 1;
    if (array_pos >= 10)
    {
        core().nonfatal("Attempt to retrieve invalid room link on " + id_str_ + " (" + to_string(array_pos) + ")", Core::CORE_ERROR);
        return nullptr;
    }
    if (!links_[array_pos]) return nullptr;
    return world().find_room(links_[array_pos]->get());
}

// Checks if an Exit exists in the specified Direction.
bool Room::has_exit(Direction dir) const
{ return (links_[link_id(dir, "has_exit", false)] != nullptr); }

// Retrieves the hashed ID of this Room.
uint32_t Room::id() const { return id_; }

// Retrieves the string ID of this Room.
const string& Room::id_str() const { return id_str_; }

// Checks if this Room has an unfinished link in a specified direction.
bool Room::is_unfinished(Direction dir) const
{
    if (dir == Direction::NONE || dir > Direction::DOWN) throw runtime_error("Invalid direction call from is_unfinished [" + id_str_ + "]");
    return tag(unfinished_directions_[static_cast<int>(dir) - 1]);
}

// Turns a Direction into an int for array access, produces a standard error on invalid input.
int Room::link_id(Direction dir, const std::string& caller, bool fail_on_null) const
{
    if (dir == Direction::NONE || dir > Direction::DOWN) throw runtime_error("Invalid direction call from " + caller + " [" + id_str_ + "]");
    int array_pos = static_cast<int>(dir) - 1;
    if (fail_on_null && !links_[array_pos]) throw runtime_error("Null link direction call from " + caller + " [" + id_str_ + "]");
    return array_pos;
}

// Checks a LinkTag on a specified Link.
bool Room::link_tag(Direction dir, LinkTag tag) const
{
    int array_pos = link_id(dir, "link_tag", true);
    return links_[array_pos]->tag(tag);
}

// Loads only the changes to this Room from a save file. Should only be called by a parent Region.
void Room::load_delta(FileReader* file)
{
    uint32_t delta_tag = 0;
    do
    {
        delta_tag = file->read_data<uint32_t>();
        switch(delta_tag)
        {
            case ROOM_DELTA_ENTITIES:
            {
                // Load any Entities in this Room.
                const size_t entity_count = file->read_data<size_t>();
                entities_.reserve(entity_count);
                for (size_t i = 0; i < entity_count; i++)
                {
                    EntityType type = file->read_data<EntityType>();
                    switch(type)
                    {
                        case EntityType::ENTITY: add_entity(std::make_unique<Entity>(file)); break;
                        case EntityType::MOBILE: add_entity(std::make_unique<Mobile>(file)); break;
                        case EntityType::PLAYER: add_entity(std::make_unique<Player>(file)); break;
                        default: throw runtime_error("Attempt to load unknown entity type: " + to_string(static_cast<int>(type)));
                    }
                }
                break;
            }

            case ROOM_DELTA_TAGS:
            {
                // Clear all existing tags, and load the full set of tags in from the save file.
                tags_.clear();
                size_t tag_count = file->read_data<size_t>();
                for (size_t i = 0; i < tag_count; i++)
                    set_tag(file->read_data<RoomTag>(), false);
                break;
            }

            case ROOM_DELTA_DESC:
            {
                // Update the room description.
                desc_ = file->read_string();
                break;
            }

            case ROOM_DELTA_LINKS:
            {
                for (int i = 0; i < 10; i++)
                {
                    const uint32_t link_delta_type = file->read_data<uint32_t>();
                    switch(link_delta_type)
                    {
                        // If no Link is marked, delete any Link that may currently be there.
                        case ROOM_DELTA_LINK_NONE: links_[i].reset(nullptr); break;

                        // If the Link is unchanged, ensure it exists, and if so, do nothing more.
                        case ROOM_DELTA_LINK_UNCHANGED:
                            if (!links_[i]) throw runtime_error("Missing link marked as unchanged! [" + id_str_ + "]");
                            break;

                        // If the Link has changed, load it from the data file, creating a blank Link first if needed.
                        case ROOM_DELTA_LINK_CHANGED:
                        {
                            if (!links_[i]) links_[i] = std::make_unique<Link>();
                            links_[i]->load_delta(file);
                            break;
                        }

                        default: FileReader::standard_error("Unknown link delta identifier", link_delta_type, 0, {id_str_});
                    }
                }
                break;
            }

            case ROOM_DELTA_NAME:
            {
                // Replace the room name with the save file data.
                name_[0] = file->read_string();
                name_[1] = file->read_string();
                break;
            }

            case ROOM_DELTA_MAP_CHAR:
            {
                // Replace the map character with the save file data.
                map_char_ = file->read_string();
                break;
            }

            case ROOM_DELTA_END: break;
            default: FileReader::standard_error("Unrecognized delta tag in room data", delta_tag, 0, {id_str_});
        }
    } while(delta_tag != ROOM_DELTA_END);
}

// Look around you. Just look around you.
void Room::look()
{
    const bool automap_enabled = !player().player_tag(PlayerTag::AutomapOff);
    const unsigned int term_width = terminal::get_width();
    const unsigned int minimap_width = (automap_enabled ? 11 : 0);
    const unsigned int desc_width = term_width - minimap_width;

    if (automap_enabled && !player().player_tag(PlayerTag::TutorialAutomap))
    {
        player().set_player_tag(PlayerTag::TutorialAutomap);
        print("{c}An automatically-generated map of the nearby area will be displayed next to the room description. If you wish to disable this feature, "
            "simply type: {C}automap off\n");
    }

    vector<string> room_desc = StrX::ansi_vector_split("  "+ desc_, desc_width);
    room_desc.insert(room_desc.begin(), "{C}" + name_[0]);

    if (can_see_outside())
    {
        vector<string> weather_desc = StrX::ansi_vector_split("{K}  " + world().time_weather().weather_desc(), desc_width);
        room_desc.insert(room_desc.end(), weather_desc.begin(), weather_desc.end());
    }

    vector<string> exits_list;
    string exits_list_str;
    for (int i = 0; i < 10; i++)
    {
        if (!links_[i]) continue;
        const uint32_t exit = links_[i]->get();
        string exit_name = "{C}" + direction_name(static_cast<Direction>(i + 1)) + "{c}";
        const Room* target_room = world().find_room(exit);

        vector<string> exit_tags;
        if (target_room->tag(RoomTag::Explored)) exit_tags.push_back(target_room->short_name());
        if (links_[i]->tag(LinkTag::Openable))
        {
            if (links_[i]->tag(LinkTag::Open)) exit_tags.push_back("open");
            else if (links_[i]->tag(LinkTag::AwareOfLock)) exit_tags.push_back("locked");
            else exit_tags.push_back("closed");
        }

        if (exit_tags.size()) exit_name += " (" + StrX::comma_list(exit_tags) + ")";
        exits_list.push_back(exit_name);
    }
    if (exits_list.size()) exits_list_str = string("  {c}There ") + (exits_list.size() > 1 ? "are " : "is ") + StrX::number_to_text(exits_list.size()) +
        " obvious exit" + (exits_list.size() > 1 ? "s" : "") + ": " + StrX::comma_list(exits_list, StrX::CL_MODE_USE_AND) + ".";
    exits_list = StrX::ansi_vector_split(exits_list_str, desc_width);
    room_desc.insert(room_desc.end(), exits_list.begin(), exits_list.end());

    // Generate the room map (if any), then combine the room map and room description together.
    vector<string> room_map, combined_vec;
    if (automap_enabled) room_map = world().automap().generate_map(this);
    const bool desc_longer = room_desc.size() > room_map.size();
    const size_t total_length = (desc_longer ? room_desc.size() : room_map.size());
    const size_t map_start = (desc_longer ? (room_desc.size() / 2 - room_map.size() / 2) : 0);
    combined_vec.reserve(total_length);
    for (size_t i = 0; i < total_length; i++)
    {
        if (i >= map_start && (i - map_start < room_map.size())) combined_vec.push_back(room_map.at(i - map_start) +
            (room_desc.size() > i ? room_desc.at(i) : ""));
        else combined_vec.push_back((automap_enabled ? "           " : "") + room_desc.at(i));
    }

    print();
    for (auto str : combined_vec)
        print(str);
}

// Retrieves the map character for this Room.
const std::string Room::map_char() const
{
    if (!map_char_.size()) throw runtime_error(id_str_ + ": empty map char");
    if (map_char_[0] == '{') return map_char_ + "{0}";
    else return "{0}" + map_char_;
}

// Retrieves the full name of this Room.
const std::string& Room::name() const { return name_[1]; }

// Parses a string RoomTag name into a RoomTag enum.
RoomTag Room::parse_room_tag(const std::string &tag)
{
    auto result = tag_map_.find(tag);
    if (result == tag_map_.end()) throw runtime_error("Invalid RoomTag: " + tag);
    return result->second;
}

// Returns the ID of the Region this Room belongs to.
uint32_t Room::region() const
{ return world().find_room_region(id_); }

// Reverses a Direction (e.g. north becomes south).
Direction Room::reverse_direction(Direction dir)
{
    if (dir > Direction::DOWN) throw runtime_error("Invalid direction call from reverse_direction");
    return reverse_direction_map_[static_cast<int>(dir)];
}

// Saves only the changes to this Room in a save file. Should only be called by a parent Region.
void Room::save_delta(FileWriter* file)
{
    // Check if anything has changed on this Room.
    const bool entities_exist = (entities_.size() > 0);
    const bool tags_changed = tag(RoomTag::ChangedTags);
    const bool desc_changed = tag(RoomTag::ChangedDesc);
    const bool exits_changed = tag(RoomTag::ChangedExits);
    const bool name_changed = tag(RoomTag::ChangedName);
    const bool map_char_changed = tag(RoomTag::ChangedMapChar);
    if (!(entities_exist || tags_changed || desc_changed || exits_changed || name_changed || map_char_changed)) return;

    // Write the save version for this Room, and the Room's ID.
    file->write_data<uint32_t>(Region::REGION_DELTA_ROOM);
    file->write_data<uint32_t>(ROOM_SAVE_VERSION);
    file->write_data<uint32_t>(id_);

    // Save any Entities in this Room.
    if (entities_exist)
    {
        file->write_data<uint32_t>(ROOM_DELTA_ENTITIES);
        file->write_data<size_t>(entities_.size());
        for (auto &entity : entities_)
            entity->save(file);
    }

    // If any tags have changed, write them all here.
    if (tags_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_TAGS);
        file->write_data<size_t>(tags_.size());
        for (auto &tag : tags_)
            file->write_data<RoomTag>(tag);
    }

    // If the room description has changed, add it here.
    if (desc_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_DESC);
        file->write_string(desc_);
    }

    // If any of the exits have changed, add them here.
    if (exits_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_LINKS);
        for (int i = 0; i < 10; i++)
        {
            if (links_[i])
            {
                if (links_[i]->changed())
                {
                    file->write_data<uint32_t>(ROOM_DELTA_LINK_CHANGED);
                    links_[i]->save_delta(file);
                }
                else file->write_data<uint32_t>(ROOM_DELTA_LINK_UNCHANGED);
            }
            else file->write_data<uint32_t>(ROOM_DELTA_LINK_NONE);
        }
    }

    // If the room's short name has changed, add it here.
    if (name_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_NAME);
        file->write_string(name_[0]);
        file->write_string(name_[1]);
    }

    // If the map character has changed, add it here.
    if (map_char_changed)
    {
        file->write_data<uint32_t>(ROOM_DELTA_MAP_CHAR);
        file->write_string(map_char_);
    }

    // Mark the end of the changes.
    file->write_data<uint32_t>(ROOM_DELTA_END);
}

// Sets the description of this Room.
void Room::set_desc(const string& new_desc, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedDesc);
    if (!new_desc.size())
    {
        core().nonfatal("Attempt to set blank description on room (" + id_str_ + ")", Core::CORE_ERROR);
        desc_ = "Missing room description.";
    }
    else desc_ = new_desc;
}

// Sets an exit link from this Room to another.
void Room::set_link(Direction dir, uint32_t new_exit, bool mark_delta)
{
    int array_pos = link_id(dir, "set_link", false);
    if (!links_[array_pos])
    {
        auto new_link = std::make_unique<Link>();
        new_link->set(new_exit, mark_delta);
        links_[array_pos] = std::move(new_link);
    }
    else links_[array_pos]->set(new_exit, mark_delta);
    if (mark_delta) set_tag(RoomTag::ChangedExits);
}

// Sets a LinkTag on a specifieid Link.
void Room::set_link_tag(Direction dir, LinkTag tag, bool mark_delta)
{
    int array_pos = link_id(dir, "set_link_tag", true);
    links_[array_pos]->set_tag(tag, mark_delta);
    if (mark_delta) set_tag(RoomTag::ChangedExits);
}

// Sets multiple LinkTags at once.
void Room::set_link_tags(Direction dir, std::list<LinkTag> tags_list, bool mark_delta)
{
    int array_pos = link_id(dir, "set_link_tags", true);
    links_[array_pos]->set_tags(tags_list, mark_delta);
    if (mark_delta) set_tag(RoomTag::ChangedExits);
}

// Sets the map character for this Room.
void Room::set_map_char(const std::string& new_char, bool mark_delta)
{
    if (mark_delta) set_tag(RoomTag::ChangedMapChar);
    map_char_ = new_char;
}

// Sets the short name of this Room.
void Room::set_name(const string& new_name, const string& new_short_name, bool mark_delta)
{
    if (!new_name.size() && !new_short_name.size()) return;
    if (mark_delta) set_tag(RoomTag::ChangedName);
    if (new_name.size()) name_[0] = new_name;
    if (new_short_name.size()) name_[1] = new_short_name;
}

// Sets a RoomTag on this Room.
void Room::set_tag(RoomTag the_tag, bool mark_delta)
{
    if (tags_.count(the_tag) > 0) return;
    tags_.insert(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Sets multiple RoomTags at the same time.
void Room::set_tags(std::list<RoomTag> tags_list, bool mark_delta)
{
    for (auto the_tag : tags_list)
        set_tag(the_tag);
    if (mark_delta) set_tag(RoomTag::ChangedTags, false);
}

// Retrieves the short name of this Room.
const string& Room::short_name() const { return name_[1]; }

// Checks if a RoomTag is set on this Room.
bool Room::tag(RoomTag the_tag) const { return (tags_.count(the_tag) > 0); }

// Transfers a specified Entity from this Room to a target Room.
void Room::transfer(Entity* entity_ptr, Room* room_ptr)
{
    // First, sanity checks. These should never happen, but could possibly occur as the result of mistakes in the code.
    if (room_ptr == this)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to itself.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer enity (" + entity_ptr->name() + ") from " + id_str_ + " to itself.", Core::CORE_ERROR);
        return;
    }
    if (!entity_ptr)
    {
        if (!room_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to " + room_ptr->id_str() + ".", Core::CORE_ERROR);
        return;
    }
    if (!room_ptr)
    {
        if (!entity_ptr) core().nonfatal("Attempt to transfer null entity from " + id_str_ + " to null room.", Core::CORE_ERROR);
        else core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to null room.", Core::CORE_ERROR);
        return;
    }
    if (entity_ptr->parent_room() != this)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            " while entity is not correctly parented to this room.", Core::CORE_ERROR);
        return;
    }

    // Try to determine which vector position houses the Entity being moved.
    uint32_t source_id = 0;
    bool source_found = false;
    for (size_t i = 0; i < entities_.size(); i++)
    {
        if (entities_.at(i).get() == entity_ptr)
        {
            source_id = i;
            source_found = true;
            break;
        }
    }
    if (!source_found)
    {
        core().nonfatal("Attempt to transfer entity (" + entity_ptr->name() + ") from " + id_str_ + " to " + room_ptr->id_str() +
            ", while entity is not contained within the parent room.", Core::CORE_ERROR);
        return;
    }

    // We now know the position in the entities_ vector for the requested entity, and all the pointers appear to be in order. Let's transfer ownership...
    room_ptr->entities_.push_back(std::move(entities_[source_id]));
    // ...and immediately delete the now-invalid pointer from our own entities_ vector...
    entities_.erase(entities_.begin() + source_id);
    // ...and finally, update the Entity's parent Room pointer.
    entity_ptr->set_parent_room(room_ptr);
}

}   // namespace westgate
