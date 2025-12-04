// util/namegen.hpp -- Procedural name generation for NPCs.

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

#pragma once
#include "core/pch.hpp" // Precompiled header

namespace westgate {

enum class Gender : uint8_t;    // defined in world/entity/entity.hpp

class ProcNameGen {
public:
    void        load_namelists();   // Loads the namelists from the data files.
    std::string npc_name(Gender gender, bool with_surname = true);  // Generates a random NPC name, using a combination of the other systems.

private:
    std::string consonant();    // Picks a consonant from the table, for forming atoms.
    std::string name_f();       // Returns a random feminine name.
    std::string name_m();       // Returns a random masculine name.
    std::string namegen_v1();   // Generates a random name (v1 code, Elite-style).
    std::string namegen_v4(const std::string& pattern, unsigned int max_len = 8, unsigned int min_len = 2); // Generates a name with the v4 generator.
    std::string pv3_t();        // Ends of words.
    std::string random_word(bool cap = false);  // Generates a random word.
    std::string surname();      // Generates a random surname.
    std::string vowel();        // Picks a vowel from the table, for forming atoms.

    std::string                 consonant_block;    // The consonant letter block, for v1 naming.
    std::vector<std::string>    names_f;            // Hard-coded names list of feminine first-names.
    std::vector<std::string>    names_m;            // Hard-coded names list of masculine first-names.
    std::vector<std::string>    names_s_a;          // Hard-coded names list, containing the first half of surnames.
    std::vector<std::string>    names_s_b;          // Hard-coded names list, containing the second half of surnames.
    std::vector<std::string>    pv3_c;              // Consonant letter block, for v3 naming.
    std::vector<std::string>    pv3_d;              // Dipthong letter block, for v3 naming.
    std::vector<std::string>    pv3_e;              // Before silent E letter block, for v3 naming.
    std::vector<std::string>    pv3_f;              // Final letters block, for v3 naming.
    std::vector<std::string>    pv3_i;              // Prefix letter block, for v3 naming.
    std::vector<std::string>    pv3_k;              // Latinate letter block, for v3 naming.
    std::vector<std::string>    pv3_v;              // Simple vowels block, for v3 naming.
    std::vector<std::string>    pv3_x;              // Final vowels block, for v3 naming.
    std::string                 v4_template;        // Template used for v4 namegen.
    std::string                 vowel_block;        // The vowel letter block, for v1 naming.
};

}   // namespace westgate
