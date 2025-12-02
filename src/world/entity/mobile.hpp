// world/entity/mobile.hpp -- A derived class of Entity, the Mobile class defines things that move around of their own volition, or things that are close
// enough. For example, this covers the player, NPCs, monsters and animals, but could also be used for things like training dummies or automatons.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include "world/entity/entity.hpp"

namespace westgate {

class FileReader;   // defined in util/filereader.cpp
class FileWriter;   // defined in util/filewriter.hpp

class Mobile : public Entity {
public:
                        Mobile() = delete;  // No default constructor; use nullptr on the constructor below.
                        Mobile(FileReader* file);   // Creates a blank Mobile, then loads its data from a FileReader.
    virtual             ~Mobile() = default;    // Virtual destructor, in case Mobile is used by a further derived class in the future.
    virtual void        save(FileWriter* file) override;    // Saves this Mobile to a save game file.
    virtual EntityType  type() const override { return EntityType::MOBILE; }    // Self-identifies this Entity's derived class.
};

}   // namespace westgate
