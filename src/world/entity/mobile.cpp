// world/entity/mobile.cpp -- A derived class of Entity, the Mobile class defines things that move around of their own volition, or things that are close
// enough. For example, this covers the player, NPCs, monsters and animals, but could also be used for things like training dummies or automatons.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "trailmix/file/filereader.hpp"
#include "trailmix/file/filewriter.hpp"
#include "world/entity/mobile.hpp"

using namespace trailmix;

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
