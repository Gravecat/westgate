// src/parser/parser.cpp -- The parser processes input from the player and converts it into commands in the game world.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <functional>
#include <map>
#include <unordered_map>

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "parser/cheats.hpp"
#include "parser/meta.hpp"
#include "parser/parser.hpp"
#include "parser/parser-macros.hpp"
#include "parser/silly.hpp"
#include "parser/world-interaction.hpp"
#include "trailmix/text/hash.hpp"
#include "trailmix/text/stringutils.hpp"
#include "world/area/room.hpp"

using namespace trailmix;
using std::stol;
using std::string;
using std::vector;
using westgate::terminal::print;

namespace westgate {
namespace parser {

static const std::map<uint32_t, Direction> parser_directions = {
    { 1081869984, Direction::NORTH }, { 4254119393, Direction::NORTH },
    { 1897786808, Direction::NORTHEAST }, { 3641754167, Direction::NORTHEAST },
    { 717260451, Direction::EAST }, { 4163295959, Direction::EAST },
    { 1457891302, Direction::SOUTHEAST }, { 3801532777, Direction::SOUTHEAST },
    { 819466240, Direction::SOUTH }, { 3627942915, Direction::SOUTH },
    { 2626121188, Direction::SOUTHWEST }, { 656258893, Direction::SOUTHWEST },
    { 3976103327, Direction::WEST }, { 3359274028, Direction::WEST },
    { 320024672, Direction::NORTHWEST }, { 4257653048, Direction::NORTHWEST },
    { 367575389, Direction::UP }, { 2399778729, Direction::UP },
    { 715181085, Direction::DOWN }, { 2573673949, Direction::DOWN },
};

static const std::unordered_map<uint32_t, std::function<void(vector<uint32_t>&, vector<string>&)>> parser_verbs = {
    { 2252282012, parser::cheats::hash },   // #hash
    { 3069208872, parser::meta::automap },  // automap
    { 2746646486, parser::world_interaction::open_close },  // close
    { 2573673949, parser::world_interaction::travel },  // d
    { 715181085, parser::world_interaction::travel },   // down
    { 4163295959, parser::world_interaction::travel },  // e
    { 717260451, parser::world_interaction::travel },   // east
    { 3693685262, parser::silly::magic_word },  // frotz
    { 93100650, parser::world_interaction::travel },    // go
    { 1214476199, parser::world_interaction::look },    // l
    { 365823675, parser::world_interaction::look }, // look
    { 3654652163, parser::meta::automap },  // map
    { 516519904, parser::meta::automap },   // minimap
    { 1337450370, parser::world_interaction::travel },  // move
    { 4254119393, parser::world_interaction::travel },  // n
    { 3641754167, parser::world_interaction::travel },  // ne
    { 1081869984, parser::world_interaction::travel },  // north
    { 1897786808, parser::world_interaction::travel },  // northeast
    { 320024672, parser:: world_interaction::travel },  // northwest
    { 4257653048, parser::world_interaction::travel },  // nw
    { 21229531, parser::world_interaction::open_close}, // open
    { 1253391317, parser::silly::magic_word },  // plugh
    { 1533866676, parser::silly::magic_word },  // plover
    { 3289483580, parser::meta::quit }, // quit
    { 3627942915, parser::world_interaction::travel },  // s
    { 1633956953, parser::meta::save }, // save
    { 3801532777, parser::world_interaction::travel },  // se
    { 819466240, parser::world_interaction::travel },   // south
    { 1457891302, parser::world_interaction::travel },  // southeast
    { 2626121188, parser::world_interaction::travel },  // southwest
    { 656258893, parser::world_interaction::travel },   // sw
    { 1908976648, parser::world_interaction::travel },  // travel
    { 2399778729, parser::world_interaction::travel },  // u
    { 367575389, parser::world_interaction::travel },   // up
    { 3359274028, parser::world_interaction::travel },  // w
    { 51785697, parser::world_interaction::wait },  // wait
    { 3976103327, parser::world_interaction::travel },  // west
    { 42193550, parser::silly::magic_word },    // xyzzy
    { 1601889381, parser::world_interaction::wait } // z
};

// Parses a hashed string into a Direction enum.
Direction parse_direction(uint32_t hash)
{
    auto result = parser_directions.find(hash);
    if (result == parser_directions.end()) return Direction::NONE;
    else return result->second;
}

// Attempts to parse a string into a number; invalid results are set to INT_MIN.
int32_t parse_number(const std::string &num)
{
    int32_t result = 0;
    try
    { result = stol(num); }
    catch(const std::invalid_argument&)
    { return INT_MIN; }
    catch(const std::out_of_range&)
    { return INT_MIN; }
    return result;
}

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
    return (terminal::get_number(0, 1, true) == 1);
}

} } // parser, westgate namespaces
