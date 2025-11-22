// src/parser/parser-macros.hpp -- Compiler macros to make the parser functions less tedious to write.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#define PARSER_FUNCTION     std::vector<uint32_t>& words_hashed, std::vector<std::string>& words
#define PARSER_NO_WORDS     (void)words;
#define PARSER_NO_HASHED    (void)words_hashed;
