// world/entity/item.cpp -- Derived from Entity, an Item is something that can be picked up and carried. This covers equipment, treasure, and many other things.

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

#include "world/entity/item.hpp"

namespace westgate {

// Creates a blank Item, then loads its data from a FileReader.
Item::Item(FileReader* file) : Entity(file)
{
    if (!file) return;
    // file loading code goes here
}

// Saves this Item to a save game file.
void Item::save(FileWriter* file) { Entity::save(file); }

}   // namespace westgate
