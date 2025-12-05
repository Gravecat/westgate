// util/yaml.cpp -- Helper functions to make using Rapid YAML easier.

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

#include "util/filex.hpp"
#include "util/yaml.hpp"

using std::runtime_error;
using std::string;
using std::string_view;
using std::vector;

namespace westgate {

// Blank constructor.
YAML::YAML() : ref_(nullptr) { }

// Calls load_file() when constructing.
YAML::YAML(string_view filename, bool allow_backslash) { load_file(filename, allow_backslash); }

// Creates a new YAML object from a parent tree.
YAML::YAML(const ryml::Tree tree, ryml::ConstNodeRef new_ref) : ref_(new_ref), tree_(tree) { }

// Retrieves a value from a sequence, as a string.
string YAML::get(size_t index) const
{
    if (!is_seq()) throw runtime_error("Not a sequence!");
    if (index >= size()) throw runtime_error("Invalid sequence index!");
    return string(noderef()[index].val().str).substr(0, noderef()[index].val().len);
}

// Retrieves a child of this tree.
YAML YAML::get_child(string_view key) const
{
    YAML new_yaml(tree_, ref_[ryml::to_csubstr(key)]);
    return new_yaml;
}

// Retrieves all values of a sequence.
vector<string> YAML::get_seq(string_view key) const
{
    if (!key_exists(key)) throw runtime_error("Missing YAML key: " + string{key});
    YAML yaml = get_child(key);
    if (!yaml.is_seq()) throw runtime_error("Invalid YAML key (not a sequence): " + string{key});
    vector<string> vec(yaml.size());
    for (size_t i = 0; i < yaml.size(); i++)
        vec.at(i) = yaml.get(i);
    return vec;
}

// Checks if the noderef points to a valid map.
bool YAML::is_map() const { return noderef().is_map(); }

// Checks if the noderef points to a valid sequence.
bool YAML::is_seq() const { return noderef().is_seq(); }

// Checks if a given key exists.
bool YAML::key_exists(string_view key) const
{
    if (!is_map()) throw runtime_error("Not a map!");
    return noderef().has_child(ryml::to_csubstr(key));
}

// Retrieves the key values of a map.
vector<string> YAML::keys() const
{
    if (!is_map()) throw runtime_error("Not a map!");
    ryml::ConstNodeRef::children_view children = noderef().children();
    vector<string> vec_out;
    for (auto child : children)
        vec_out.push_back(string(child.key().str).substr(0, child.key().len));
    return vec_out;
}

// Retrieves the key/value pairs of a map.
std::map<string, string> YAML::keys_vals() const
{
    if (!is_map()) throw runtime_error("Not a map!");
    ryml::ConstNodeRef::children_view children = noderef().children();
    std::map<string, string> map_out;
    for (auto child : children)
    {
        string key_str = (string(child.key().str).substr(0, child.key().len));
        if (!child.has_val()) throw runtime_error("No values!");
        string val_str = (string(child.val().str).substr(0, child.val().len));
        map_out.insert(std::pair<string, string>(key_str, val_str));
    }
    return map_out;
}

// Loads a YAML file into memory and parse it.
void YAML::load_file(std::string_view filename, bool allow_backslash)
{
    string file_string = FileX::file_to_string(string{filename}.c_str());
    // If we don't care about using backslash for... whatever rapidYAML does with them, just turn them into double-backslashes so they're treated as a
    // string literal of \ instead of... I don't know, it's probably used for writing hex or octal or some shit.
    if (!allow_backslash)
    {
        for (size_t i = 0; i < file_string.size(); i++)
        {
            if (file_string[i] == '\\')
            {
                file_string = file_string.substr(0, i) + '\\' + file_string.substr(i);
                i++;
            }
        }
    }
    tree_ = ryml::parse_in_arena(ryml::to_csubstr(file_string));
    ref_ = tree_.rootref();
}

// Returns the noderef for the loaded tree.
ryml::ConstNodeRef YAML::noderef() const { return ref_; }

// Checks the number of children on the noderef.
size_t YAML::size() const { return static_cast<size_t>(noderef().num_children()); }

// Returns the value of a key, as a string.
string YAML::val(string_view key) const
{
    auto cskey = ryml::to_csubstr(key);
    return string(noderef()[cskey].val().str).substr(0, noderef()[cskey].val().len);
}

}   // namespace westgate
