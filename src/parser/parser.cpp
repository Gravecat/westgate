// src/parser/parser.cpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <functional>
#include <map>

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "parser/cheats.hpp"
#include "parser/meta.hpp"
#include "parser/parser.hpp"
#include "parser/parser-macros.hpp"
#include "parser/silly.hpp"
#include "parser/world.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"

using std::string;
using std::vector;
using westgate::terminal::print;

namespace westgate {
namespace parser {

static const std::map<uint32_t, std::function<void(vector<uint32_t>&, vector<string>&)>> parser_verbs = {
    { 2252282012, parser::cheats::hash },       // #hash
    { 3693685262, parser::silly::magic_word },  // frotz
    { 1214476199, parser::world::look },        // l
    { 365823675, parser::world::look },         // look
    { 1253391317, parser::silly::magic_word },  // plugh
    { 1533866676, parser::silly::magic_word },  // plover
    { 3289483580, parser::meta::quit },         // quit
    { 1633956953, parser::meta::save },         // save
    { 42193550, parser::silly::magic_word }     // xyzzy
};

// Processes input from the player.
void process_input(const string& input)
{
    if (!input.size()) return;  // Nothing to do here.

    // Split the input into multiple words. We're mostly gonna be using a 'verb noun' structure here, but it might get more complex later.
    vector<string> words = stringutils::string_explode(input);
    vector<uint32_t> word_hashes;
    for (auto word : words)
        word_hashes.push_back(hash::murmur3(stringutils::str_tolower(word)));

    auto result = parser_verbs.find(word_hashes.at(0));
    if (result == parser_verbs.end()) print("{Y}I don't know that word.");
    else result->second(word_hashes, words);
}

// Displays a yes/no prompt for the player, returns their choice.
bool yes_no()
{
    print("Please select one of the following options:");
    print("{K}[{G}0{K}] {w}No");
    print("{K}[{G}1{K}] {w}Yes");
    return (terminal::get_number(0, 1) == 1);
}

} } // parser, westgate namespaces
