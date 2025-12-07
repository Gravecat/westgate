// world/entity/entity.cpp -- The Entity class defines "things" in the game world, primarily used for derived classes Actor and Item, but may be used for other
// derived classes in the future. Entity does NOT define the "rooms" of the game world, only "things" within it.

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

#include <cctype>

#include "core/core.hpp"
#include "util/filex.hpp"
#include "world/area/room.hpp"
#include "world/entity/entity.hpp"
#include "world/entity/inventory.hpp"
#include "world/entity/item.hpp"
#include "world/entity/mobile.hpp"
#include "world/entity/player.hpp"

using std::runtime_error;
using std::string;
using std::string_view;
using std::to_string;

namespace westgate {

// Creates a blank Entity, then loads its data from a FileReader.
Entity::Entity(FileReader* file) : gender_(Gender::NONE), name_("undefined entity"), parent_entity_(nullptr), parent_room_(nullptr)
{
    if (!file) return;

    // Check the save version for this Entity.
    if (const unsigned int save_version = file->read_data<unsigned int>();
        save_version != ENTITY_SAVE_VERSION) FileReader::standard_error("Invalid entity save version", save_version, ENTITY_SAVE_VERSION);

    // Retrieve the Entity's name and gender.
    if (const unsigned int props_tag = file->read_data<unsigned int>();
        props_tag != ENTITY_SAVE_PROPS) FileReader::standard_error("Invalid tag in entity save data", props_tag, ENTITY_SAVE_PROPS);
    name_ = file->read_string();
    gender_ = file->read_data<Gender>();

    // Load the Entity's tags, if any.
    if (const unsigned int tags_tag = file->read_data<unsigned int>();
        tags_tag != ENTITY_SAVE_TAGS) FileReader::standard_error("Invalid tag in entity save data", tags_tag, ENTITY_SAVE_TAGS);
    size_wg tag_count = file->read_data<size_wg>();
    for (size_wg t = 0; t < tag_count; t++)
        set_tag(file->read_data<EntityTag>());

    // Load the Entity's Inventory, if any.
    if (const unsigned int inv_tag = file->read_data<unsigned int>();
        inv_tag != ENTITY_SAVE_INVENTORY) FileReader::standard_error("Invalid tag in entity save data", inv_tag, ENTITY_SAVE_INVENTORY);
    if (file->read_data<bool>()) inventory_ = std::make_unique<Inventory>(file);
}

// Virtual destructor. Nothing here yet, but this needs to be defined *here* and not inline in entity.hpp
Entity::~Entity() = default;

// Adds an Inventory to this Entity if it doesn't already have one.
void Entity::add_inventory()
{
    if (inventory_) inventory_->clear();
    else inventory_ = std::make_unique<Inventory>();
}

// Clears an EntityTag from this Entity.
void Entity::clear_tag(EntityTag the_tag)
{
    if (!(tags_.count(the_tag) > 0)) return;
    tags_.erase(the_tag);
}

// Clears multiple EntityTags at the same time.
void Entity::clear_tags(std::list<EntityTag> tags_list) { for (auto the_tag : tags_list) clear_tag(the_tag); }

// Retrieves the gender (if any) of this Entity.
Gender Entity::gender() const { return gender_; }

// Returns a gender string (he/she/it/they/etc.)
const string Entity::he_she(bool capitalize_first) const
{
    switch(gender_)
    {
        case Gender::SHE: return (capitalize_first ? "She" : "she");
        case Gender::HE: return (capitalize_first ? "He" : "he");
        case Gender::IT: return (capitalize_first ? "It" : "it");
        case Gender::THEY: return (capitalize_first ? "They" : "they");
        default: return (capitalize_first ? "It" : "it");
    }
}

// Returns a gender string (himself/herself/theirself/etc.)
const string Entity::himself_herself() const
{
    switch(gender_)
    {
        case Gender::SHE: return "herself";
        case Gender::HE: return "himself";
        case Gender::IT: return "itself";
        case Gender::THEY: return "theirself";
        default: return "itself";
    }
}

// Returns a gender string (his/her/its/their/etc.)
const string Entity::his_her() const
{
    switch(gender_)
    {
        case Gender::SHE: return "her";
        case Gender::HE: return "his";
        case Gender::IT: return "its";
        case Gender::THEY: return "their";
        default: return "its";
    }
}

// Returns a pointer to the attached Inventory, if any.
Inventory* Entity::inv() { return inventory_.get(); }

// Loads an Entity from a File, returning a unique_ptr to the appropriate class.
std::unique_ptr<Entity> Entity::load_entity(FileReader* file)
{
    if (!file) throw runtime_error("Attempt to load Entity from null file pointer!");
    switch(EntityType type = file->read_data<EntityType>())
    {
        case EntityType::ENTITY: return std::make_unique<Entity>(file); break;
        case EntityType::MOBILE: return std::make_unique<Mobile>(file); break;
        case EntityType::PLAYER: return std::make_unique<Player>(file); break;
        case EntityType::ITEM: return std::make_unique<Item>(file); break;
        default: throw runtime_error("Attempt to load unknown entity type: " + to_string(static_cast<int>(type)));
    }
}

// Retrieves the name of this Entity.
const string Entity::name(unsigned int flags) const
{
    const bool the = ((flags & NAME_FLAG_THE) == NAME_FLAG_THE);
    const bool capitalize_first = ((flags & NAME_FLAG_CAPITALIZE_FIRST) == NAME_FLAG_CAPITALIZE_FIRST);
    const bool possessive = ((flags & NAME_FLAG_POSSESSIVE) == NAME_FLAG_POSSESSIVE);
    const bool plural = ((flags & NAME_FLAG_PLURAL) == NAME_FLAG_PLURAL);

    string ret = name_;
    if (!ret.size())
    {
        core().nonfatal("Missing mobile name!", Core::CORE_ERROR);
        return "";
    }
    if (the && !tag(EntityTag::ProperNoun)) ret = "the " + name_;
    if (capitalize_first) ret[0] = std::toupper(ret[0]);
    if (possessive)
    {
        if (ret.back() == 's') ret += "'";
        else ret += "'s";
    }
    else if (plural && ret.back() != 's')
    {
        if (ret.back() == 'h') ret += "es";
        else ret += "s";
    }

    return ret;
}

// Retrieves the Entity (if any) containing this Entity.
Entity* Entity::parent_entity() const { return parent_entity_; }

// Retrieves the Room (if any) containing this Entity.
Room* Entity::parent_room() const { return parent_room_; }

// Removes an Inventory pointer from this Entity.
void Entity::remove_inventory()
{
    if (!inventory_) core().nonfatal("Attempt to remove non-existent Inventory from Entity [" + name_ + "]", Core::CORE_ERROR);
    inventory_.reset(nullptr);
}

// Saves this Entity to a save game file.
void Entity::save(FileWriter* file)
{
    // Write this Entity's type identifier. This will be critical when loading Entities later.
    file->write_data<EntityType>(type());

    // Write the save version for this Entity.
    file->write_data<unsigned int>(ENTITY_SAVE_VERSION);

    // Write the Entity's name and gender.
    file->write_data<unsigned int>(ENTITY_SAVE_PROPS);
    file->write_string(name_);
    file->write_data<Gender>(gender_);

    // Write the Entity's tags, if any.
    file->write_data<unsigned int>(ENTITY_SAVE_TAGS);
    file->write_data<size_wg>(tags_.size());
    for (auto &tag : tags_)
        file->write_data<EntityTag>(tag);

    // Save this Entity's Inventory, if any.
    file->write_data<unsigned int>(ENTITY_SAVE_INVENTORY);
    if (inventory_)
    {
        file->write_data<bool>(true);
        inventory_->save(file);
    }
    else file->write_data<bool>(false);
}

// Sets the gender of this Entity.
void Entity::set_gender(Gender new_gender)
{
    // While unlikely, it can't hurt to check and ensure the value is within valid bounds.
    if (static_cast<unsigned char>(new_gender) > static_cast<unsigned char>(Gender::IT))
    {
        core().nonfatal("Attempt to set invalid gender (" + to_string(static_cast<unsigned char>(new_gender)) + ") on " + name_, Core::CORE_ERROR);
        new_gender = Gender::NONE;
    }
    gender_ = new_gender;
}

// Sets the name of this Entity.
void Entity::set_name(const string_view new_name) { name_ = new_name; }

// Sets a new Entity as the parent of this Entity, or nullptr for none.
void Entity::set_parent_entity(Entity* new_entity_parent)
{
    if (new_entity_parent == this)
    {
        core().nonfatal("Attempt to set entity parent to itself (" + name_ + ")", Core::CORE_ERROR);
        new_entity_parent = nullptr;
    }
    parent_entity_ = new_entity_parent;
    if (new_entity_parent) set_parent_room(nullptr);    // An Entity can only have one parent.
}

// Sets a new Room as the parent of this Entity, or nullptr for none.
void Entity::set_parent_room(Room* new_room_parent)
{
    parent_room_ = new_room_parent;
    if (new_room_parent) set_parent_entity(nullptr);    // An Entity can only have one parent.
}

// Sets an EntityTag on this Entity.
void Entity::set_tag(EntityTag the_tag)
{
    if (tags_.count(the_tag) > 0) return;
    tags_.insert(the_tag);
}

// Sets multiple EntityTags at the same time.
void Entity::set_tags(std::list<EntityTag> tags_list) { for (auto the_tag : tags_list) set_tag(the_tag); }

// Checks if an EntityTag is set on this Entity.
bool Entity::tag(EntityTag the_tag) const { return (tags_.count(the_tag) > 0); }

// Toggles an EntityTag on or off.
void Entity::toggle_tag(EntityTag the_tag)
{
    if (!tag(the_tag)) set_tag(the_tag);
    else clear_tag(the_tag);
}

}   // namespace westgate
