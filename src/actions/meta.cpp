// actions/meta.cpp -- "Meta" commands, i.e. commands that interact with the game itself, such as saving or quitting the game.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#include "actions/meta.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "parser/parser.hpp"
#include "world/entity/player.hpp"

using westgate::terminal::print;

namespace westgate::actions::meta {

// Toggles the automap on or off.
void automap(PARSER_FUNCTION)
{ PARSER_NO_WORDS
    if (words_hashed.size() < 2) player().toggle_player_tag(PlayerTag::AutomapOff);
    else
    {
        if (words_hashed.at(1) == 2768884862) player().set_player_tag(PlayerTag::AutomapOff);           // off
        else if (words_hashed.at(1) == 1246816877) player().clear_player_tag(PlayerTag::AutomapOff);    // on
        else
        {
            print("{Y}I don't understand that. Please specify \"{G}automap on{Y}\" or \"{G}automap off{Y}\".");
            return;
        }
    }
    if (player().player_tag(PlayerTag::AutomapOff)) print("{C}Automap disabled.");
    else print("{C}Automap enabled.");
}

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

}   // namespace westgate::actions::meta
