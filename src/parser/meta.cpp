// parser/meta.cpp -- "Meta" commands, i.e. commands that interact with the game itself, such as saving or quitting the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "parser/meta.hpp"
#include "parser/parser.hpp"

using westgate::terminal::print;

namespace westgate {
namespace parser {
namespace meta {

// Quits the game.
void quit(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    print("Are you sure you want to quit the game? {R}Your progress will not be saved.");
    if (parser::yes_no())
    {
        print("{B}Farewell!");
        core().destroy_core(EXIT_SUCCESS);
    }
    print("{B}Your adventure continues...");
}

// Saves the game.
void save(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    game().save(true);
}

} } }   // meta, parser, westgate namespaces
