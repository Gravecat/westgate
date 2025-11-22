// src/parser/parser.cpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <cstdlib>
#include <functional>
#include <map>
#include <vector>

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "parser/cheats.hpp"
#include "parser/meta.hpp"
#include "parser/parser.hpp"
#include "parser/parser-macros.hpp"
#include "parser/silly.hpp"
#include "util/text/hash.hpp"
#include "util/text/stringutils.hpp"

namespace westgate {
namespace parser {

static const std::map<uint32_t, std::function<void(std::vector<uint32_t>&, std::vector<std::string>&)>> parser_verbs = {
    { 2252282012, cheats::hash },       // #hash
    { 3693685262, silly::magic_word },  // frotz
    { 1253391317, silly::magic_word },  // plugh
    { 1533866676, silly::magic_word },  // plover
    { 3289483580, meta::quit },         // quit
    { 42193550, silly::magic_word }     // xyzzy
};

// Processes input from the player.
void process_input(const std::string& input)
{
    if (!input.size()) return;  // Nothing to do here.

    // Split the input into multiple words. We're mostly gonna be using a 'verb noun' structure here, but it might get more complex later.
    std::vector<std::string> words = stringutils::string_explode(input);
    std::vector<uint32_t> word_hashes;
    for (auto word : words)
        word_hashes.push_back(hash::murmur3(stringutils::str_tolower(word)));

    auto result = parser_verbs.find(word_hashes.at(0));
    if (result == parser_verbs.end()) terminal::print("{Y}I don't know that word.");
    else result->second(word_hashes, words);
}

} } // parser, westgate namespaces
