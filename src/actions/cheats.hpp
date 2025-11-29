// actions/cheats.hpp -- Cheating/development/testing commands.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp" // Precompiled header

#include "parser/parser-macros.hpp"

namespace westgate::actions::cheats {

void    hash(PARSER_FUNCTION);  // Hashes words into integers.
void    weather_desc(PARSER_FUNCTION);  // Retrieves a specified weather-map description.

}   // namespace westgate::actions::cheats
