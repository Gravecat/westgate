// util/yaml.hpp -- Helper functions to make using Rapid YAML easier.

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
#include "core/pch.hpp" // precompiled header

#include <map>

#include "3rdparty/rapidyaml/rapidyaml-0.10.0.hpp"

namespace westgate {

class YAML {
public:
                    YAML();                                     // Blank constructor.
                    YAML(const std::string_view filename, bool allow_backslash = false);    // Calls load_file() when constructing.
    std::string     get(size_t index) const;                    // Retrieves a value from a sequence, as a string.
    YAML            get_child(const std::string_view key) const;    // Retrieves a child noderef of this tree.
    std::vector<std::string>    get_seq(const std::string_view key) const;  // Retrieves all values of a sequence.
    bool            is_map() const;                             // Checks if the noderef points to a valid map.
    bool            is_seq() const;                             // Checks if the noderef points to a valid sequence.
    bool            key_exists(const std::string_view key) const;   // Checks if a given key exists.
    std::vector<std::string>    keys() const;                   // Retrieves the key values of a map.
    std::map<std::string, std::string>  keys_vals() const;      // Retrieves the key/value pairs of a map.
    void            load_file(const std::string_view filename, bool allow_backslash = false);   // Loads a YAML file into memory and parse it.
    size_t          size() const;                               // Checks the number of children on the noderef.
    std::string     val(const std::string_view key) const;      // Returns the value of a key, as a string.

protected:
                    YAML(const ryml::Tree tree, ryml::ConstNodeRef new_ref);    // Creates a new YAML object from a parent tree.

private:
    ryml::ConstNodeRef  noderef() const;    // Returns the noderef for the loaded tree.

    ryml::ConstNodeRef  ref_;   // The NodeRef for this part of the tree.
    ryml::Tree          tree_;  // The parsed YAML data.
};

}   // namespace westgate
