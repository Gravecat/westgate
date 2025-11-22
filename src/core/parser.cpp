// src/core/parser.cpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/parser.hpp"
#include "core/terminal.hpp"

namespace westgate {
namespace parser {

// Processes input from the player.
void process_input(const std::string& input)
{ terminal::print("{R}" + input); }

} } // parser, westgate namespaces
