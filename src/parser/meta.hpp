// parser/meta.hpp -- "Meta" commands, i.e. commands that interact with the game itself, such as saving or quitting the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "parser/parser.hpp"
#include "parser/parser-macros.hpp"

namespace westgate {
namespace parser {
namespace meta {

void    quit(PARSER_FUNCTION);  // Quits the game.

} } }   // meta, parser, westgate namespaces
