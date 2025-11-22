// parser/world.cpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/terminal.hpp"
#include "parser/world.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"

namespace westgate {
namespace parser {
namespace world {

// Look around you. Just look around you.
void look(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    player().parent_room()->look();
}

} } }   // world, parser, westgate namespaces
