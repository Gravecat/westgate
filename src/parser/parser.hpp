// src/parser/parser.hpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <string>

namespace westgate {
namespace parser {

void    process_input(const std::string& input);    // Processes input from the player.

} } // parser, westgate namespaces
