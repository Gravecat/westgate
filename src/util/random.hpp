// util/random.hpp -- Simple interface code to use effolkronium's random number generator.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "3rdparty/random/random.hpp"

namespace westgate {

// get base random alias which is auto seeded and has static API and internal state
using rnd = effolkronium::random_static;

}   // namespace westgate
