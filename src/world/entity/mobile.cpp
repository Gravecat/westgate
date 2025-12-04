// world/entity/mobile.cpp -- A derived class of Entity, the Mobile class defines things that move around of their own volition, or things that are close
// enough. For example, this covers the player, NPCs, monsters and animals, but could also be used for things like training dummies or automatons.

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

#include "util/filex.hpp"
#include "world/entity/mobile.hpp"

namespace westgate {

// Creates a blank Mobile, then loads its data from a FileReader.
Mobile::Mobile(FileReader* file) : Entity(file)
{
    name_ = "undefined mobile";

    if (!file) return;
    // file loading code goes here
}

// Saves this Mobile to a save game file.
void Mobile::save(FileWriter* file) { Entity::save(file); }

}   // namespace westgate
