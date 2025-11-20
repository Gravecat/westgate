// world/entity/mobile.cpp -- A derived class of Entity, the Mobile class defines things that move around of their own volition, or things that are close
// enough. For example, this covers the player, NPCs, monsters and animals, but could also be used for things like training dummies or automatons.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "world/entity/mobile.hpp"

namespace lom {

// Creates a blank Mobile with default values.
Mobile::Mobile() : Entity()
{ name_ = "undefined mobile"; }

}   // namespace lom
