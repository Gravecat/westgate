// world/entity/inventory.hpp -- The Inventory class is a collection of Items all stored in one place, and handles things such as sorting the Items, merging
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

#pragma once
#include "core/pch.hpp" // precompiled header

namespace westgate {

class Item;         // defined in world/entity/item.hpp
class FileReader;   // defined in util/filex.hpp
class FileWriter;   // defined in util/filex.cpp

class Inventory {
public:
            Inventory() = default;  // A blank constructor will make an empty Inventory, ready to go.
            Inventory(FileReader* file);    // Creates a blank Inventory, then loads it from the specified file.
    void    add(std::unique_ptr<Item> item);    // Adds an Item to this Inventory (use std::move).
    Item*   at(size_t index);       // Returns a pointer to a specified Item in this Inventory.
    void    clear();                // Deletes everything from this Inventory.
    bool    empty() const;          // Returns true if this Inventory is empty.
    void    erase(size_t index);    // Removes an Item from this Inventory.
    void    save(FileWriter* file); // Saves this Entity to a save game file.
    size_t  size() const;           // Returns the amount of Items in this Inventory.
    void    transfer(Inventory* new_inv, size_t index); // Moves an item from this Inventory into another.

private:
    static constexpr unsigned int   INVENTORY_SAVE_VERSION =    1;  // The expected version for saving/loading binary game data.

    std::vector<std::unique_ptr<Item>>  items_; // The Items stored in this Inventory.
};

}   // namespace westgate
