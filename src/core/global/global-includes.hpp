// core/global/global-includes.hpp -- Very frequently-used system headers which are included in all other headers in this project, via core/global.hpp

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS     1
#endif
#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__  1
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES       1
#endif
#ifdef LOM_TARGET_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     1
#endif
#endif

#include <climits>      // definitions such as UINT_MAX
#include <cstdint>      // typedefs such as uint32_t
#include <memory>       // std::unique_ptr is used frequently in this project
#include <stdexcept>    // standard exception types, such as std::runtime_error
#include <string>       // std::string
#include <vector>       // std::vector
