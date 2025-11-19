// core/global/enums.hpp -- Enums which are used frequently enough in the code to belong in a gloabl header.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>

namespace lom {

// The genders of things in the game.
enum class Gender : uint8_t { NONE, HE, SHE, THEY, IT };

}   // namespace lom
