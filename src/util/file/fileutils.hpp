// util/file/fileutils.hpp -- Various utility functions that deal with reading and writing files.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "core/global.hpp"

namespace lom {
namespace fileutils {

std::string file_to_string(const std::string &filename);    // Loads a text file into an std::string.

} } // fileutils, lom namespaces
