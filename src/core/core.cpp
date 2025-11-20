// core/core.cpp -- Main program entry, initialization and cleanup routines, error-handling and logging, along with pointers to the key subsystems of the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <csignal>
#include <cstdlib>
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
#include "util/file/binpath.hpp"
#include "util/file/yaml.hpp"

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
    const std::string err_buffer = stderr_buffer_.str();
    if (err_buffer.size())
    {
        this->log(err_buffer);
        stderr_buffer_.str(std::string());
        stderr_buffer_.clear();
    }
    lock_stderr_ = false;
}

// Cleans up all Core-managed objects.
void Core::cleanup()
{
    std::cout << rang::style::reset;    // Reset any lingering ANSI codes.
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
    signal(SIGABRT, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
#ifdef INVICTUS_TARGET_LINUX
    signal(SIGBUS, SIG_IGN);
#endif

    this->log("The rest is silence.");
    syslog_.close();
}

// Returns a reference to the singleton Core object.
Core& Core::core()
{
    static Core the_core;
    return the_core;
}

// Returns the full path to a specified game data file.
std::string Core::datafile(const std::string file)
{
    if (!gamedata_location_.size()) throw std::runtime_error("Could not locate valid gamedata folder!");
    return BinPath::merge_paths(gamedata_location_, file);
}

// Destroys the singleton Core object and ends execution.
void Core::destroy_core(int exit_code)
{
    if (exit_code == EXIT_SUCCESS) this->log("Normal core shutdown requested.");
    else if (exit_code == EXIT_FAILURE) this->log("Emergency core shutdown requested.", Core::CORE_CRITICAL);
    else this->log("Core shutdown with unknown error code: " + std::to_string(exit_code), Core::CORE_ERROR);
    cleanup();
    std::exit(exit_code);
}

// Attempts to locate the gamedata folder.
void Core::find_gamedata()
{
    const std::string game_path_data = BinPath::game_path("gamedata");
    const std::string game_path_data_westgate_yml = BinPath::merge_paths(game_path_data, "westgate.yml");
    const std::string source_path_data = BinPath::merge_paths(source::SOURCE_DIR, "gamedata");
    const std::string source_path_data_westgate_yml = BinPath::merge_paths(source_path_data, "westgate.yml");
    
    if (std::filesystem::exists(game_path_data_westgate_yml))
    {
        log("Game data folder location: " + game_path_data);
        gamedata_location_ = game_path_data;
    }
    else if (std::filesystem::exists(source_path_data_westgate_yml))
    {
        log("Game data folder location: " + source_path_data);
        gamedata_location_ = source_path_data;
    }
    else throw std::runtime_error("Could not locate valid gamedata folder!");

    YAML yaml_file(datafile("westgate.yml"));
    if (!yaml_file.is_map() || !yaml_file.key_exists("westgate_gamedata_version")) throw std::runtime_error("westgate.yml: Invalid file format!");
    const int data_version = std::stoi(yaml_file.val("westgate_gamedata_version"));
    if (data_version != WESTGATE_GAMEDATA_VERSION) this->halt("Unexpected gamedata version! (" + std::to_string(data_version) + ", expected " +
        std::to_string(WESTGATE_GAMEDATA_VERSION) + ")");
}

// Returns a reference to the Game manager object.
Game& Core::game() const
{
    if (!game_ptr_) throw std::runtime_error("Attempt to access null Game pointer!");
    return *game_ptr_;
}

// Used internally only to apply the most powerful possible method to kill the process, in event of emergency.
void Core::great_googly_moogly_its_all_gone_to_shit()
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
void Core::halt(std::string error)
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
void Core::halt(const std::exception &e) { halt(e.what()); }

// Hook system failure signals.
void Core::hook_signals()
{
    this->log("Error-handling system hooking signals...");
    if (signal(SIGABRT, core_intercept_signal) == SIG_ERR) halt("Failed to hook abort signal.");
    if (signal(SIGSEGV, core_intercept_signal) == SIG_ERR) halt("Failed to hook segfault signal.");
    if (signal(SIGILL, core_intercept_signal) == SIG_ERR) halt("Failed to hook illegal instruction signal.");
    if (signal(SIGFPE, core_intercept_signal) == SIG_ERR) halt("Failed to hook floating-point exception signal.");
#ifdef INVICTUS_TARGET_LINUX
    if (signal(SIGBUS, core_intercept_signal) == SIG_ERR) halt("Failed to hook bus error signal.");
#endif
}

// Sets up the core game classes and data, and the terminal subsystem.
void Core::init_core(std::vector<std::string> parameters)
{
    // Boosts performance on C++ console output (at the cost of not being able to reliably use C console output, e.g. printf, between flushes). This is
    // especially important for MinGW builds, but should provide a small boost to performance on other platforms too.
    std::ios_base::sync_with_stdio(false);
    std::cout.tie(nullptr);

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
}

// Catches a segfault or other fatal signal.
void Core::intercept_signal(int sig)
{
    std::string sig_type;
    switch(sig)
    {
        case SIGABRT: sig_type = "Software requested abort."; break;
        case SIGFPE: sig_type = "Floating-point exception."; break;
        case SIGILL: sig_type = "Illegal instruction."; break;
        case SIGSEGV: sig_type = "Segmentation fault."; break;
#ifdef INVICTUS_TARGET_LINUX
        case SIGBUS: sig_type = "Bus error."; break;
#endif
        default: sig_type = "Intercepted unknown signal."; break;
    }

    // Disable the signals for now, to stop a cascade.
    signal(SIGABRT, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
#ifdef INVICTUS_TARGET_LINUX
    signal(SIGBUS, SIG_IGN);
#endif
    halt(sig_type);
}

// Logs a message in the system log file.
void Core::log(std::string msg, int type)
{
    if (!syslog_.is_open()) return;
    if (!lock_stderr_) check_stderr();

    std::string txt_tag;
    switch(type)
    {
        case CORE_INFO: break;
        case CORE_WARN: txt_tag = "[WARN] "; std::cout << rang::bgB::yellow << rang::fg::black; break;
        case CORE_ERROR: txt_tag = "[ERROR] "; std::cout << rang::bgB::red << rang::fg::black; break;
        case CORE_CRITICAL: txt_tag = "[CRITICAL] "; std::cout << rang::bg::red << rang::fg::black; break;
    }

    char* buffer = new char[32];
    const time_t now = time(nullptr);
#if defined(WESTGATE_TARGET_WINDOWS) && !defined(WESTGATE_TARGET_MINGW)
    tm time_struct;
    tm* ptm = &time_struct;
    localtime_s(ptm, &now);
#else
    const tm *ptm = localtime(&now);
#endif
    std::strftime(&buffer[0], 32, "%H:%M:%S", ptm);
    std::string time_str = &buffer[0];
    msg = "[" + time_str + "] " + txt_tag + msg;
    syslog_ << msg << std::endl;
    delete[] buffer;

    if (type != CORE_INFO) std::cout << msg << rang::style::reset << std::endl;
}

// Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.
void Core::nonfatal(std::string error, int type)
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
    const std::filesystem::path userdata_path = BinPath::game_path("userdata");
    const std::filesystem::path logfile_path = userdata_path.string() + "/log.txt";
    if (!std::filesystem::exists(userdata_path)) std::filesystem::create_directory(userdata_path);
    if (std::filesystem::exists(logfile_path)) std::filesystem::remove(logfile_path);
    syslog_.open(logfile_path);
    if (!syslog_.is_open()) throw std::runtime_error("Cannot open " + logfile_path.string());
    hook_signals();
    stderr_old_ = std::cerr.rdbuf(stderr_buffer_.rdbuf());
    this->log("Welcome to Westgate " + version::VERSION_STRING + " build " + version::BUILD_TIMESTAMP);
    this->log("Logging and error-handling system is online.");
}

// A shortcut to using Core::core().
Core& core() { return Core::core(); }

}   // namespace westgate

// Main program entry point. Must be OUTSIDE the westgate namespace.
int main(int argc, char** argv)
{
    // Create the main Core object.
    std::vector<std::string> parameters(argv + 1, argv + argc);
    
    try { westgate::core().init_core(parameters); }
    catch (std::exception &e)
    {
        std::cout << "[FATAL] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    try
    { westgate::game().begin(); }
    catch (std::exception &e) { westgate::core().halt(e); }

    // Trigger cleanup code.
    westgate::core().destroy_core(EXIT_SUCCESS);
    return EXIT_SUCCESS;    // Technically not needed, as destroy_core() calls exit(), but this'll keep the compiler happy.
}
