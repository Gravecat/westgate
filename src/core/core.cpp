// core/core.cpp -- Main program entry, initialization and cleanup routines, error-handling and logging, along with pointers to the key subsystems of the game.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE, std::getenv

#include "core/core.hpp"
#include "cmake/version.hpp"
#include "util/file/binpath.hpp"

#include <csignal>
#include <filesystem>

namespace lom {

// This has to be a non-class function because C.
void core_intercept_signal(int sig) { core().intercept_signal(sig); }

// Constructor, sets up the Core object.
Core::Core() : cascade_count_(0), cascade_failure_(false), cascade_timer_(std::time(0)), dead_already_(0), lock_stderr_(false), stderr_old_(nullptr) { }

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
    close_log();
    std::cout << style::reset;   // Reset any lingering ANSI codes.
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
    (void)parameters;
    open_log();
    terminal::set_window_title("Lom v" + version::VERSION_STRING + " (" + version::BUILD_TIMESTAMP + ")");
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
        case CORE_WARN: txt_tag = "[WARN] "; std::cout << bgB::yellow << fg::black; break;
        case CORE_ERROR: txt_tag = "[ERROR] "; std::cout << bgB::red << fg::black; break;
        case CORE_CRITICAL: txt_tag = "[CRITICAL] "; std::cout << bg::red << fg::black; break;
    }

    char* buffer = new char[32];
    const time_t now = time(nullptr);
#if defined(LOM_TARGET_WINDOWS) && !defined(LOM_TARGET_MINGW)
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

    if (type != CORE_INFO) std::cout << msg << EOL;
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
    this->log("Welcome to Lom " + version::VERSION_STRING + " build " + version::BUILD_TIMESTAMP);
    this->log("Logging and error-handling system is online.");
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
    
    try { core().init_core(parameters); }
    catch (std::exception &e)
    {
        std::cout << "[FATAL] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
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
            }

#ifdef LOM_TARGET_WINDOWS
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

        // Test code
        std::cout << fgB::cyan << "Hello, world!" << EOL;
    }
    catch (std::exception &e) { core().halt(e); }

    // Trigger cleanup code.
    core().destroy_core(EXIT_SUCCESS);
    return EXIT_SUCCESS;    // Technically not needed, as destroy_core() calls exit(), but this'll keep the compiler happy.
}
