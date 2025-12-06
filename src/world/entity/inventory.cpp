// world/entity/inventory.cpp -- The Inventory class is a collection of Items all stored in one place, and handles things such as sorting the Items, merging
// identical Items together into a single stack, etc.

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

#include "core/core.hpp"
#include "util/filex.hpp"
#include "world/entity/inventory.hpp"
#include "world/entity/item.hpp"

using std::runtime_error;
using std::to_string;
using std::unique_ptr;

namespace westgate {

// Creates a blank Inventory, then loads it from the specified file.
Inventory::Inventory(FileReader* file)
{
    if (!file)
    {
        core().nonfatal("Called Inventory(FileReader*) with null pointer. Use Inventory() instead.", Core::CORE_WARN);
        return;
    }

    // Check the save version for this Inventory.
    const unsigned int save_version = file->read_data<unsigned int>();
    if (save_version != INVENTORY_SAVE_VERSION) FileReader::standard_error("Invalid inventory save version", save_version, INVENTORY_SAVE_VERSION);

    // Read the size of this Inventory.
    const size_wg inv_size = file->read_data<size_wg>();
    items_.reserve(inv_size);

    // Iterate over the Inventory, loading each Entity within.
    for (size_wg i = 0; i < inv_size; i++)
    {
        unique_ptr<Entity> ent = Entity::load_entity(file);
        if (ent->type() != EntityType::ITEM) throw runtime_error("Non-Item Entity loaded from within Inventory data: " + ent->name());

        // Safely cast the Entity (which we now *know* is an Item) into a std::unique_ptr<Item>
        Item* item_ptr = dynamic_cast<Item*>(ent.get());
        if (!item_ptr) throw runtime_error("Unable to cast Entity to Item!");
        
        // Release ownership on the old unique_ptr, and push a new unique_ptr into the vector.
        ent.release();
        items_.emplace_back(item_ptr);
    }
}

// Adds an Item to this Inventory (use std::move).
void Inventory::add(unique_ptr<Item> item)
{
    items_.push_back(std::move(item));
    // eventually, we'll handle things like stacking identical items here
}

// Returns a pointer to a specified Item in this Inventory.
Item* Inventory::at(size_t index)
{
    if (index >= items_.size())
        throw runtime_error("Attempt to access invalid Item index in Inventory (" + to_string(index) + ", size " + to_string(items_.size()));
    return items_.at(index).get();
}

// Deletes everything from this Inventory.
void Inventory::clear() { items_.clear(); }

// Removes an Item from this Inventory.
void Inventory::erase(size_t index)
{
    if (index >= items_.size())
        throw runtime_error("Attempt to erase invalid Item index in Inventory (" + to_string(index) + ", size " + to_string(items_.size()));
    items_.erase(items_.begin() + index);
}

// Saves this Entity to a save game file.
void Inventory::save(FileWriter* file)
{
    // Write the save version for this Inventory.
    file->write_data<unsigned int>(INVENTORY_SAVE_VERSION);

    // Write the size of the Inventory.
    file->write_data<size_wg>(items_.size());

    // Iterate over the Inventory, saving each Entity within.
    for (auto &item : items_)
        item->save(file);
}

// Returns the amount of Items in this Inventory.
size_t Inventory::size() const { return items_.size(); }

// Moves an item from this Inventory into another.
void Inventory::transfer(Inventory* new_inv, size_t index)
{
    if (!new_inv) throw runtime_error("Attempt to transfer item into null inventory.");
    if (index >= items_.size())
        throw runtime_error("Attempt to transfer invalid Item index in Inventory (" + to_string(index) + ", size " + to_string(items_.size()));
    new_inv->add(std::move(items_.at(index)));
    items_.erase(items_.begin() + index);
}

}   // namespace westgate
