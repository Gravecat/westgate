// core/core.cpp -- Main program entry, initialization and cleanup routines, along with pointers to the key subsystems of the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE, std::getenv

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "cmake/version.hpp"

#ifdef LOM_TARGET_WINDOWS
#include <windows.h>
#endif
#ifdef LOM_TARGET_LINUX
#include <csignal>
#endif

namespace lom {

// Constructor, sets up the Core object.
Core::Core() { }

// Cleans up all Core-managed objects.
void Core::cleanup() { }

// Returns a reference to the singleton Core object.
Core& Core::core()
{
    static Core the_core;
    return the_core;
}

// Destroys the singleton Core object and ends execution.
void Core::destroy_core(int exit_code)
{
    //if (exit_code == EXIT_SUCCESS) log("Normal core shutdown requested.");
    //else if (exit_code == EXIT_FAILURE) log("Emergency core shutdown requested.", Core::CORE_CRITICAL);
    //else log("Core shutdown with unknown error code: " + std::to_string(exit_code), Core::CORE_ERROR);
    cleanup();
    std::exit(exit_code);
}

// Used internally only to apply the most powerful possible method to kill the process, in event of emergency.
void Core::great_googly_moogly_its_all_gone_to_shit()
{
#ifdef LOM_TARGET_WINDOWS
    TerminateProcess(GetCurrentProcess(), 1);
#else   // LOM_TARGET_WINDOWS
#if LOM_TARGET_LINUX
    std::raise(SIGKILL);
#else   // LOM_TARGET_LINUX
    std::terminate();   // Not great, but that's our fallback.
#endif  // LOM_TARGET_LINUX
#endif  // LOM_TARGET_WINDOWS
}

// Sets up the core game classes and data, and the terminal subsystem.
void Core::init_core(std::vector<std::string> parameters)
{
    (void)parameters;
}

// A shortcut to using Core::core().
Core& core() { return Core::core(); }

}   // namespace lom

// Main program entry point. Must be OUTSIDE the lom namespace.
int main(int argc, char** argv)
{
    using namespace lom;

    // Create the main Core object.
    std::vector<std::string> parameters(argv + 1, argv + argc);

#ifdef LOM_TARGET_WINDOWS
    // Check if lom.exe is already running.
    //lom::Process::check_if_already_running();
#endif
    
    try { core().init_core(parameters); }
    catch (std::exception &e)
    {
        std::cout << "[FATAL] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Check command-line parameters.
    if (parameters.size()) { }

    // Test code
    terminal::set_window_title("Lom v" + version::VERSION_STRING + " (" + version::BUILD_TIMESTAMP + ")");
    cout << fg::cyan << style::bold << "Hello, world!" << style::reset << endl;

    // Trigger cleanup code.
    core().destroy_core(EXIT_SUCCESS);
    return EXIT_SUCCESS;    // Technically not needed, as destroy_core() calls exit(), but this'll keep the compiler happy.
}
