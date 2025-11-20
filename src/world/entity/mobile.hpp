// world/entity/mobile.hpp -- A derived class of Entity, the Mobile class defines things that move around of their own volition, or things that are close
// enough. For example, this covers the player, NPCs, monsters and animals, but could also be used for things like training dummies or automatons.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "world/entity/entity.hpp"

namespace westgate {

class Mobile : public Entity {
public:
            Mobile();   // Creates a blank Mobile with default values.
    virtual ~Mobile() = default;    // Virtual destructor, in case Mobile is used by a further derived class in the future.

private:
};

}   // namespace westgate
