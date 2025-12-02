// util/binpath.cpp -- Some very nasty platform-specific code for determining the path of the binary file at runtime.
// Based on and modified from code I found on StackOverflow: https://stackoverflow.com/questions/1528298/get-path-of-executable

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstring>

#ifdef WESTGATE_TARGET_WINDOWS
#include <windows.h>
#include <psapi.h>      // EnumProcesses(), GetModuleBaseName()
#include <shlwapi.h>    // PathRemoveFileSpecA(), PathCombineA()
#endif

#ifdef WESTGATE_TARGET_LINUX
#include <libgen.h>         // dirname()
#include <linux/limits.h>   // PATH_MAX
#if defined(__sun)
    #define PROC_SELF_EXE "/proc/self/path/a.out"
#else
    #define PROC_SELF_EXE "/proc/self/exe"
#endif
#endif

#ifdef WESTGATE_TARGET_APPLE
#include <libgen.h>         // dirname()
#include <mach-o/dyld.h>    // _NSGetExecutablePath()
#endif

#include "util/binpath.hpp"

using std::string;

namespace westgate {

string BinPath::exe_dir;   // The path to the binary.

// Given a path or filename, combines it with the current executable path and returns the combined, full path. At least, that's the hope.
string BinPath::game_path(const string &path)
{
    if (!exe_dir.size()) exe_dir = get_executable_dir();
    return merge_paths(exe_dir, path);
}

#ifdef WESTGATE_TARGET_WINDOWS
// Platform-agnostic way to find this binary's runtime path.
string BinPath::get_executable_path()
{
    char rawPathName[MAX_PATH];
    GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
    return string(rawPathName);
}

string BinPath::get_executable_dir()
{
    string executablePath = get_executable_path();
    char* exePath = new char[executablePath.length() + 1];
#ifdef WESTGATE_TARGET_MINGW
    strcpy(exePath, executablePath.c_str());
#else
    strcpy_s(exePath, executablePath.length() + 1, executablePath.c_str());
#endif
    PathRemoveFileSpecA(exePath);
    string directory = string(exePath);
    delete[] exePath;
    return directory;
}

// Merges two path strings together.
string BinPath::merge_paths(const string &pathA, const string &pathB)
{
    char combined[MAX_PATH];
    PathCombineA(combined, pathA.c_str(), pathB.c_str());
    string merged_path(combined);
    if (merged_path.size() && merged_path[0] == '/')    // Special check for Windows WSL, where it may be a Linux path.
        std::replace(merged_path.begin(), merged_path.end(), '\\', '/');
    else    // If not, replace any / in the paths with \.
        std::replace(merged_path.begin(), merged_path.end(), '/', '\\');
    return merged_path;
}
#endif  // WESTGATE_TARGET_WINDOWS

#ifdef WESTGATE_TARGET_LINUX
string BinPath::get_executable_path()
{
    char rawPathName[PATH_MAX];
    char* rp = realpath(PROC_SELF_EXE, rawPathName);
    if (rp) return string(rawPathName);
    else return "";
}

string BinPath::get_executable_dir()
{
    const string& executablePath = get_executable_path();
    char *executablePathStr = new char[executablePath.length() + 1];
    strcpy(executablePathStr, executablePath.c_str());
    char* executableDir = dirname(executablePathStr);
    const string exec_dir = string(executableDir);
    delete[] executablePathStr;
    return exec_dir;
}

string BinPath::merge_paths(const string &pathA, const string &pathB) { return pathA + "/" + pathB; }
#endif  // WESTGATE_TARGET_LINUX

#ifdef WESTGATE_TARGET_APPLE
string BinPath::get_executable_path()
{
    char rawPathName[PATH_MAX];
    char realPathName[PATH_MAX];
    uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);

    char* rp = nullptr;
    if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) rp = realpath(rawPathName, realPathName);
    if (rp) return string(realPathName);
    else return "";
}

string BinPath::get_executable_dir()
{
    const string& executablePath = get_executable_path();
    char *executablePathStr = new char[executablePath.length() + 1];
    strcpy(executablePathStr, executablePath.c_str());
    char* executableDir = dirname(executablePathStr);
    const string exec_dir = string(executableDir);
    delete[] executablePathStr;
    return exec_dir;
}

string BinPath::merge_paths(const string& pathA, const string& pathB) { return pathA + "/" + pathB; }
#endif  // WESTGATE_TARGET_APPLE

}   // namespace westgate
