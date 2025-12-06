// core/core.cpp -- Main program entry, initialization and cleanup routines, error-handling and logging, along with pointers to the key subsystems of the game.

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

#include <csignal>
#include <iostream>
#include <filesystem>

#ifdef WESTGATE_TARGET_WINDOWS
#include <windows.h>
#endif

#include "3rdparty/rang/rang.hpp"
#include "cmake/source.hpp"
#include "cmake/version.hpp"
#include "core/core.hpp"
#include "core/game.hpp"
#include "core/terminal.hpp"
#include "util/filex.hpp"
#include "util/strx.hpp"
#include "util/timer.hpp"
#include "util/yaml.hpp"

using std::exception;
using std::runtime_error;
using std::string;
using std::string_view;
using std::to_string;
using std::vector;
namespace fs = std::filesystem;

namespace westgate {

// This has to be a non-class function because C.
void core_intercept_signal(int sig) { core().intercept_signal(sig); }

// Constructor, sets up the Core object.
Core::Core() : cascade_count_(0), cascade_failure_(false), cascade_timer_(std::time(0)), dead_already_(0), lock_stderr_(false), stderr_old_(nullptr),
    game_ptr_(nullptr) { }

// Checks stderr for any updates, puts them in the log if any exist.
void Core::check_stderr()
{
    if (lock_stderr_) return;
    
    lock_stderr_ = true;
    const string err_buffer = stderr_buffer_.str();
    if (err_buffer.size())
    {
        this->log(err_buffer);
        stderr_buffer_.str(string());
        stderr_buffer_.clear();
    }
    lock_stderr_ = false;
}

// Cleans up all Core-managed objects.
void Core::cleanup()
{
    std::cout << rang::style::reset << '\n';    // Reset any lingering ANSI codes.
    std::cout.flush();  // Ensure anything left on the console output buffer (including the reset code we just added) is flushed.

    // Release all attached objects.
    game_ptr_.reset(nullptr);

    close_log();    // Close the log file.
}

// Closes the system log and releases hooks.
void Core::close_log()
{
    check_stderr();
    this->log("Logging and error-handling system shutting down.");
    if (stderr_old_)
    {
        std::cerr.rdbuf(stderr_old_);
        stderr_old_ = nullptr;
    }

    // Drop all signal hooks.
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGINT, SIG_DFL);
#ifdef INVICTUS_TARGET_LINUX
    signal(SIGBUS, SIG_DFL);
#endif

    this->log("Be seeing you!");
    syslog_.close();
}

// Returns a reference to the singleton Core object.
Core& Core::core()
{
    static Core the_core;
    return the_core;
}

// Returns the full path to a specified game data file.
const string Core::datafile(const string_view file)
{
    if (!gamedata_location_.size()) throw runtime_error("Could not locate valid gamedata folder!");
    return FileX::merge_paths(gamedata_location_, file);
}

// Destroys the singleton Core object and ends execution.
void Core::destroy_core(int exit_code)
{
    if (exit_code == EXIT_SUCCESS) this->log("Normal core shutdown requested.");
    else if (exit_code == EXIT_FAILURE) this->log("Emergency core shutdown requested.", Core::CORE_CRITICAL);
    else this->log("Core shutdown with unknown error code: " + to_string(exit_code), Core::CORE_ERROR);
    cleanup();
    std::exit(exit_code);
}

// Attempts to locate the gamedata folder.
void Core::find_gamedata()
{
    const string game_path_data = FileX::game_path("gamedata");
    const string game_path_data_westgate_yml = FileX::merge_paths(game_path_data, "westgate.yml");
    const string source_path_data = FileX::merge_paths(source::SOURCE_DIR, "gamedata");
    const string source_path_data_westgate_yml = FileX::merge_paths(source_path_data, "westgate.yml");
    
    if (fs::exists(game_path_data_westgate_yml))
    {
        log("Game data folder location: " + game_path_data);
        gamedata_location_ = game_path_data;
    }
    else if (fs::exists(source_path_data_westgate_yml))
    {
        log("Game data folder location: " + source_path_data);
        gamedata_location_ = source_path_data;
    }
    else throw runtime_error("Could not locate valid gamedata folder!");

    YAML yaml_file(datafile("westgate.yml"));
    if (!yaml_file.is_map() || !yaml_file.key_exists("westgate_gamedata_version")) throw runtime_error("westgate.yml: Invalid file format!");
    const uint32_t data_version = std::stoul(yaml_file.val("westgate_gamedata_version"));
    if (data_version != WESTGATE_GAMEDATA_VERSION) this->halt("Unexpected gamedata version! (" + to_string(data_version) + ", expected " +
        to_string(WESTGATE_GAMEDATA_VERSION) + ")");
}

// Returns a reference to the Game manager object.
Game& Core::game() const
{
    if (!game_ptr_) throw runtime_error("Attempt to access null Game pointer!");
    return *game_ptr_;
}

         /*
    /\__/__/\
   /         \
 \(ﾐ  ⌒ ● ⌒ */
void Core::great_googly_moogly_its_all_gone_to_shit()   // Applies the most powerful possible method to kill the process, in event of emergency.
{
#ifdef WESTGATE_TARGET_WINDOWS
    TerminateProcess(GetCurrentProcess(), 1);
#elif defined(WESTGATE_TARGET_LINUX)
    std::raise(SIGKILL);
#else
    std::terminate();   // Not great, but that's our fallback.
#endif
    exit(EXIT_FAILURE); // This is just overkill. :3
}

// Stops the game and displays an error messge.
void Core::halt(const string_view error)
{
    check_stderr();
    this->log("Critical error occurred, halting execution.", CORE_CRITICAL);
    this->log(error, CORE_CRITICAL);
    if (dead_already_)
    {
        if (dead_already_ > 1) great_googly_moogly_its_all_gone_to_shit();
        log("Detected cleanup in process, attempting to die peacefully.", CORE_WARN);
        dead_already_ = 2;
        destroy_core(EXIT_FAILURE);
    }
    dead_already_ = 1;
    destroy_core(EXIT_FAILURE);
}

// As above, but with an exception instead of a string.
void Core::halt(const exception& e) { halt(e.what()); }

// (ง ͠° ͟ʖ ͡°)ง ᴛʜɪs ɪs ᴏᴜʀ ᴛᴏwɴ, sᴄʀᴜʙ. (ง ͠° ͟ʖ ͡°)ง
// (ง •̀_•́)ง ʏᴇᴀʜ, ʙᴇᴀᴛ ɪᴛ! (ง •̀_•́)ง
void Core::hook_signals()   // Hook system failure signals.
{
    this->log("Error-handling system hooking signals...");
    if (signal(SIGABRT, core_intercept_signal) == SIG_ERR) halt("Failed to hook abort signal.");
    if (signal(SIGSEGV, core_intercept_signal) == SIG_ERR) halt("Failed to hook segfault signal.");
    if (signal(SIGILL, core_intercept_signal) == SIG_ERR) halt("Failed to hook illegal instruction signal.");
    if (signal(SIGFPE, core_intercept_signal) == SIG_ERR) halt("Failed to hook floating-point exception signal.");
    if (signal(SIGINT, core_intercept_signal) == SIG_ERR) halt("Failed to hook interrupt signal.");
#ifdef INVICTUS_TARGET_LINUX
    if (signal(SIGBUS, core_intercept_signal) == SIG_ERR) halt("Failed to hook bus error signal.");
#endif
}

// Sets up the core game classes and data, and the terminal subsystem.
void Core::init_core(vector<string> parameters)
{
    Timer init_timer;
    open_log(); // Creates and opens the log.txt file.
    bool set_title = rang::rang_implementation::supportsColor();

    // Check command-line parameters.
    for (auto param : parameters)
    {
        if (param == "-no-colour" || param == "-no-color")
        {
            core().log("Disabling ANSI colour codes.");
            rang::setControlMode(rang::control::Off);
        }
        else if (param == "-force-colour" || param == "-force-color")
        {
            core().log("Force-enabling ANSI colour codes.");
            rang::setControlMode(rang::control::Force);
            set_title = true;
        }

#ifdef WESTGATE_TARGET_WINDOWS
        else if (param == "-native")
        {
            core().log("Forcing use of native console attributes.");
            rang::setWinTermMode(rang::winTerm::Native);
        }
        else if (param == "-ansi")
        {
            core().log("Forcing use of ANSI console attributes.");
            rang::setWinTermMode(rang::winTerm::Ansi);
        }
#endif
    }

    if (set_title) terminal::set_window_title("Westgate v" + version::VERSION_STRING + " (" + version::BUILD_TIMESTAMP + ")");
    find_gamedata();
    game_ptr_ = std::make_unique<Game>();
#ifdef WESTGATE_BUILD_DEBUG
    this->log("Core initialized in " + StrX::ftos(init_timer.elapsed() / 1000.0f, 3) + " seconds.");
#endif
}

// Catches a segfault or other fatal signal.
void Core::intercept_signal(int sig)
{
    string sig_type;
    switch(sig)
    {
        case SIGABRT: sig_type = "Software requested abort."; break;
        case SIGFPE: sig_type = "Floating-point exception."; break;
        case SIGILL: sig_type = "Illegal instruction."; break;
        case SIGSEGV: sig_type = "Segmentation fault."; break;
        case SIGINT: destroy_core(EXIT_SUCCESS); break;
#ifdef INVICTUS_TARGET_LINUX
        case SIGBUS: sig_type = "Bus error."; break;
#endif
        default: sig_type = "Intercepted unknown signal."; break;
    }

    // Disable the signals for now, to stop a cascade.
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGINT, SIG_DFL);
#ifdef INVICTUS_TARGET_LINUX
    signal(SIGBUS, SIG_DFL);
#endif
    halt(sig_type);
}

// Logs a message in the system log file.
void Core::log(const string_view msg, int type)
{
    if (!syslog_.is_open()) return;
    if (!lock_stderr_) check_stderr();

    string txt_tag;
    switch(type)
    {
        case CORE_INFO: break;
        case CORE_WARN: txt_tag = "[WARN] "; std::cout << rang::bgB::yellow << rang::fg::black; break;
        case CORE_ERROR: txt_tag = "[ERROR] "; std::cout << rang::bgB::red << rang::fg::black; break;
        case CORE_CRITICAL: txt_tag = "[CRITICAL] "; std::cout << rang::bg::red << rang::fg::black; break;
    }

    char* buffer = new char[32];
    const time_t now = std::time(nullptr);
#if defined(WESTGATE_TARGET_WINDOWS) && !defined(WESTGATE_TARGET_MINGW)
    tm time_struct;
    tm* ptm = &time_struct;
    localtime_s(ptm, &now);
#else
    const tm *ptm = std::localtime(&now);
#endif
    std::strftime(&buffer[0], 32, "%H:%M:%S", ptm);
    string time_str = &buffer[0], msg_str = string{msg};
    msg_str = "[" + time_str + "] " + txt_tag + msg_str;
    syslog_ << msg_str << std::endl;
    delete[] buffer;

    if (type != CORE_INFO) std::cout << msg_str << rang::style::reset << std::endl;
}

// Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.
void Core::nonfatal(const string_view error, int type)
{
    if (cascade_failure_ || dead_already_) return;
    int cascade_weight = 0;
    switch(type)
    {
        case CORE_WARN: cascade_weight = ERROR_CASCADE_WEIGHT_WARNING; break;
        case CORE_ERROR: cascade_weight = ERROR_CASCADE_WEIGHT_ERROR; break;
        case CORE_CRITICAL: cascade_weight = ERROR_CASCADE_WEIGHT_CRITICAL; break;
        default: nonfatal("Nonfatal error reported with incorrect severity specified.", CORE_WARN); break;
    }

    this->log(error, type);

    if (cascade_weight)
    {
        time_t elapsed_seconds = std::time(0) - cascade_timer_;
        if (elapsed_seconds <= ERROR_CASCADE_TIMEOUT)
        {
            cascade_count_ += cascade_weight;
            if (cascade_count_ > ERROR_CASCADE_THRESHOLD)
            {
                cascade_failure_ = true;
                halt("Cascade failure detected!");
            }
        }
        else
        {
            cascade_timer_ = std::time(0);
            cascade_count_ = 0;
        }
    }
}

// Opens the output log for messages.
void Core::open_log()
{
    const fs::path userdata_path = FileX::game_path("userdata");
    const fs::path logfile_path = userdata_path.string() + "/log.txt";
    if (!fs::exists(userdata_path)) fs::create_directory(userdata_path);
    if (fs::exists(logfile_path)) fs::remove(logfile_path);
    syslog_.open(logfile_path);
    if (!syslog_.is_open()) throw runtime_error("Cannot open " + logfile_path.string());
    this->log("Welcome to Westgate " + version::VERSION_STRING + " build " + version::BUILD_TIMESTAMP);
    hook_signals();
    stderr_old_ = std::cerr.rdbuf(stderr_buffer_.rdbuf());
    this->log("Logging and error-handling system is online.");
}

// A shortcut to using Core::core().
Core& core() { return Core::core(); }

}   // namespace westgate

// Main program entry point. Must be OUTSIDE the westgate namespace.
int main(int argc, char** argv)
{
    // Create the main Core object.
    vector<string> parameters(argv + 1, argv + argc);
    
    try { westgate::core().init_core(parameters); }
    catch (exception& e)
    {
        std::cout << "[FATAL] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    try
    { westgate::game().begin(); }
    catch (exception& e) { westgate::core().halt(e); }

    // Trigger cleanup code.
    westgate::core().destroy_core(EXIT_SUCCESS);
    return EXIT_SUCCESS;    // Technically not needed, as destroy_core() calls exit(), but this'll keep the compiler happy.
}
