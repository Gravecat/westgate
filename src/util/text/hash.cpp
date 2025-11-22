// util/text/hash.cp -- Simple wrapper code to provide much simpler access to MurmurHash3, for hashing simple strings into integers.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "3rdparty/murmurhash3/MurmurHash3.h"
#include "util/text/hash.hpp"

using std::string;

namespace westgate {
namespace hash {

// Hashes a string with MurmurHash3.
uint32_t murmur3(const string& str)
{
    const uint32_t seed = 0x9747b28c;   // Arbitrary seed, but changing this will break save files.
    uint32_t hash = 0;  // Shouldn't matter, but I don't like uninitialized variables on principle.
    MurmurHash3_x86_32(str.data(), static_cast<int>(str.size()), seed, &hash);
    return hash;
}

} } // hash, westgate namespaces
