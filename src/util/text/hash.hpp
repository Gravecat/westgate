// util/text/hash.hpp -- Simple wrapper code to provide much simpler access to MurmurHash3, for hashing simple strings into integers.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include <cstdint>
#include <string>

namespace westgate {
namespace hash {

uint32_t    murmur3(const std::string& str);   // Hashes a string with MurmurHash3.

} }   // hash, westgate namespaces
