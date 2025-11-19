// util/math/random.hpp -- Simple interface code to use effolkronium's random number generator.
// Raine "Gravecat" Simmons, 2025.

#pragma once

#include "3rdparty/random/random.hpp"

namespace lom {

// get base random alias which is auto seeded and has static API and internal state
using random = effolkronium::random_static;

}   // namespace lom
