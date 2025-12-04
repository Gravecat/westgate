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

#include "world/entity/inventory.hpp"
#include "world/entity/item.hpp"

using std::runtime_error;
using std::to_string;

namespace westgate {

// Adds an Item to this Inventory (use std::move).
void Inventory::add(std::unique_ptr<Item> item)
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

// Removes an Item from this Inventory.
void Inventory::erase(size_t index)
{
    if (index >= items_.size())
        throw runtime_error("Attempt to erase invalid Item index in Inventory (" + to_string(index) + ", size " + to_string(items_.size()));
    items_.erase(items_.begin() + index);
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
