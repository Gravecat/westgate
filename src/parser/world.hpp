// parser/world.hpp -- Commands that interact with the world, in ways that aren't specific enough to get their own source files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "parser/parser.hpp"
#include "parser/parser-macros.hpp"

namespace westgate {
namespace parser {
namespace world {

void    look(PARSER_FUNCTION);  // Look around you. Just look around you.

} } }   // world, parser, westgate namespaces
