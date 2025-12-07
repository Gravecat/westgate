// actions/player-interaction.cpp -- Actions that involve the player interacting with or getting data about themselves, such as checking inventory.

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

#include "actions/player-interaction.hpp"
#include "core/terminal.hpp"
#include "util/strx.hpp"
#include "world/entity/inventory.hpp"
#include "world/entity/item.hpp"
#include "world/entity/player.hpp"

namespace westgate::actions::player_interaction {

using std::string;
using std::vector;
using westgate::terminal::print;

// Checks what items the player is carrying.
void inventory(PARSER_FUNCTION)
{ PARSER_NO_WORDS PARSER_NO_HASHED
    Inventory* inv = player().inv();
    if (inv->empty())
    {
        print("You aren't carrying anything.");
        return;
    }

    const size_t inv_size = inv->size();
    vector<string> item_names(inv_size);
    for (size_t i = 0; i < inv_size; i++)
        item_names.at(i) = inv->at(i)->name();

    print("You are carrying " + strx::comma_list(item_names, strx::CL_MODE_USE_AND) + ".");
}

}  // namespace westgate::actions::player_interaction
