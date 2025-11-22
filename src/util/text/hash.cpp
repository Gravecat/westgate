// util/text/hash.cp -- Simple wrapper code to provide much simpler access to MurmurHash3, for hashing simple strings into integers.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifdef WESTGATE_BUILD_DEBUG
#include <map>
#include "core/core.hpp"
#endif

#include "3rdparty/murmurhash3/MurmurHash3.h"
#include "util/text/hash.hpp"

using std::string;
using std::to_string;

namespace westgate {
namespace hash {

// Hashes a string with MurmurHash3.
uint32_t murmur3(const string& str)
{
    const uint32_t seed = 0x9747b28c;   // Arbitrary seed, but changing this will break save files.
    uint32_t hash = 0;  // Shouldn't matter, but I don't like uninitialized variables on principle.
    MurmurHash3_x86_32(str.data(), static_cast<int>(str.size()), seed, &hash);

#ifdef WESTGATE_BUILD_DEBUG
    check_hash_collision(str, hash);
#endif

    return hash;
}

// Only in debug builds, we're gonna add some extra code to detect hash collisions in real-time. Yes, it'll slow performance by a tiny amount, but it's a
// debug build, we're not expecting maximum optimization and speed here.
#ifdef WESTGATE_BUILD_DEBUG
std::map<uint32_t, string> backward_hash_map;

void check_hash_collision(const string& str, uint32_t hash)
{
    auto result_b = backward_hash_map.find(hash);
    if (result_b == backward_hash_map.end())
    {
        backward_hash_map.insert({hash, str});
        return;
    }
    if (!result_b->second.compare(str)) return;
    core().nonfatal("Hash collision detected! " + str + " and " + result_b->second + " both hash to " + to_string(hash), Core::CORE_WARN);
}
#endif  // WESTGATE_BUILD_DEBUG

} } // hash, westgate namespaces
