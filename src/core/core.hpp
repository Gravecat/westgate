// core/core.hpp -- Main program entry, initialization and cleanup routines, error-handling and logging, along with pointers to the key subsystems of the game.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once
#include "core/pch.hpp" // Precompiled header

#include <ctime>
#include <fstream>
#include <sstream>

namespace westgate {

class Game; // defined in core/game.hpp

class Core {
public:
    static constexpr int    CORE_INFO =     0;  // General logging information.
    static constexpr int    CORE_WARN =     1;  // Warnings, non-fatal stuff.
    static constexpr int    CORE_ERROR =    2;  // Serious errors. Shit is going down.
    static constexpr int    CORE_CRITICAL = 3;  // Critical system failure.

    void                check_stderr();                 // Checks stderr for any updates, puts them in the log if any exist.
    static Core&        core();                         // Returns a reference to the singleton Core object.
    const std::string   datafile(const std::string_view file);  // Returns the full path to a specified game data file.
    void                destroy_core(int exit_code);    // Destroys the singleton Core object and ends execution.
    void                find_gamedata();                // Attempts to locate the gamedata folder.
    Game&               game() const;                   // Returns a reference to the Game manager object.
    void                halt(const std::string_view error); // Stops the game and displays an error messge.
    void                halt(const std::exception &e);  // As above, but with an exception instead of a string.
    void                init_core(std::vector<std::string> parameters);     // Sets up the core game classes and data, and the terminal subsystem.
    void                intercept_signal(int sig);      // Catches a segfault or other fatal signal.
    void                log(const std::string_view msg, int type = CORE_INFO);  // Logs a message in the system log file.
                        // Reports a non-fatal error, which will be logged but won't halt execution unless it cascades.
    void                nonfatal(const std::string_view error, int type);

private:
    static constexpr int        ERROR_CASCADE_THRESHOLD =       25; // The amount cascade_count can reach within CASCADE_TIMEOUT seconds before it aborts.
    static constexpr int        ERROR_CASCADE_TIMEOUT =         30; // The number of seconds without an error to reset the cascade timer.
    static constexpr int        ERROR_CASCADE_WEIGHT_CRITICAL = 20; // The amount a critical type log entry will add to the cascade timer.
    static constexpr int        ERROR_CASCADE_WEIGHT_ERROR =    5;  // The amount an error type log entry will add to the cascade timer.
    static constexpr int        ERROR_CASCADE_WEIGHT_WARNING =  1;  // The amount a warning type log entry will add to the cascade timer.
    static constexpr uint32_t   WESTGATE_GAMEDATA_VERSION =     1;  // The expected version for the gamedata folder.

    int                 cascade_count_;     // Keeps track of rapidly-occurring, non-fatal error messages.
    bool                cascade_failure_;   // Is a cascade failure in progress?
    time_t              cascade_timer_;     // Timer to check the speed of non-halting warnings, to prevent cascade locks.
    uint8_t             dead_already_;      // Have we already died? Is this crash within the Core subsystem?
    std::string         gamedata_location_; // The path of the game's data files.
    bool                lock_stderr_;       // Whether the stderr-checking code is allowed to run or not.
    std::stringstream   stderr_buffer_;     // Pointer to a stringstream buffer used to catch stderr messages.
    std::streambuf*     stderr_old_;        // The old stderr buffer.
    std::ofstream       syslog_;            // The system log file.

    std::unique_ptr<Game>   game_ptr_;      // Pointer to the Game manager object, which handles the current game state.

            Core();         // Constructor, sets up the Core object.
    void    cleanup();      // Attempts to gracefully clean up memory and subsystems.
    void    close_log();    // Closes the system log and releases hooks.
    void    great_googly_moogly_its_all_gone_to_shit(); // Applies the most powerful possible method to kill the process, in event of emergency.
    void    hook_signals(); // Hook system failure signals.
    void    open_log();     // Opens the output log for messages.

};  // class Core

Core&   core(); // A shortcut to using Core::core().

}   // namespace westgate
