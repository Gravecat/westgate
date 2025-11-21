// core/terminal.cpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <functional>
#include <iostream>
#include <vector>

#if defined(WESTGATE_TARGET_LINUX) || defined(WESTGATE_TARGET_APPLE)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef WESTGATE_TARGET_WINDOWS
#include <windows.h>
#endif

#include "3rdparty/rang/rang.hpp"
#include "core/core.hpp"
#include "core/terminal.hpp"

namespace westgate {
namespace terminal {

    // Gets the width of the console window, in characters.
unsigned int get_width()
{
#ifdef WESTGATE_TARGET_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

// Prints a string of text with std::cout, processing ANSI colour tags.
void print(const std::string& text)
{
    const unsigned int console_width = get_width();
    unsigned int chars_so_far = 0;

    std::function<void(std::string)> print_formatted = [&print_formatted, &console_width, &chars_so_far](const std::string &line)
    {
        if (!line.size()) return;

        // If newlines are specified, break them up and handle them with recursive calls.
        auto new_line = line.find_first_of('\n');
        if (new_line != std::string::npos)
        {
            const std::string before_newline = line.substr(0, new_line);
            const std::string after_newline = line.substr(new_line + 1);
            print_formatted(before_newline);
            std::cout << '\n';
            chars_so_far = 0;
            print_formatted(after_newline);
            return;
        }

        // If there's any spaces, print one word at a time.
        auto space = line.find_first_of(' ');
        if (space == std::string::npos)
        {
            std::cout << line;
            chars_so_far += line.size();
            if (chars_so_far >= console_width) chars_so_far -= console_width;   // Make the best guess for how many characters ended up on the next line.
            return;
        }
        const std::string before_space = line.substr(0, space);
        const std::string after_space = line.substr(space + 1);
        bool space_after = false;
        if (chars_so_far + before_space.size() > console_width)
        {
            std::cout << '\n';
            chars_so_far = 0;
        }
        if (after_space.size()) space_after = true;
        std::cout << before_space;
        chars_so_far += before_space.size();
        if (chars_so_far == console_width)
        {
            std::cout << '\n';
            chars_so_far = 0;
        }
        else if (space_after)
        {
            std::cout << ' ';
            chars_so_far++;
        }
        if (before_space.size() >= console_width) chars_so_far -= console_width;
        print_formatted(after_space);
    };

    std::string output = text;
    std::vector<std::string> invalid_tags;
    while(output.size())
    {
        auto opener_pos = output.find_first_of('{');    // Try to find an opening tag for an ANSI colour.
        auto closer_pos = output.find_first_of('}');    // And a matching closing tag.
        if (opener_pos == std::string::npos || closer_pos == std::string::npos) // If none are found, just print the string as-is and finish.
        {
            print_formatted(output);
            break;
        }

        // Clip out the text before, during, and after the colour tag.
        std::string untagged = output.substr(0, opener_pos);
        std::string tag = output.substr(opener_pos + 1, closer_pos - opener_pos - 1);
        output = output.substr(closer_pos + 1);

        // Print any text before the next ANSI tag. If there's an existing tag already in play, it'll cause this text to be coloured.
        if (untagged.size()) std::cout << untagged;

        // Process the ANSI codes from the tag.
        bool invalid_tag = false;
        if (tag.size() >= 1) switch(tag[0])
        {
            case 'k': std::cout << rang::fg::black; break;
            case 'r': std::cout << rang::fg::red; break;
            case 'g': std::cout << rang::fg::green; break;
            case 'y': std::cout << rang::fg::yellow; break;
            case 'b': std::cout << rang::fg::blue; break;
            case 'm': std::cout << rang::fg::magenta; break;
            case 'c': std::cout << rang::fg::cyan; break;
            case 'w': std::cout << rang::fg::gray; break;
            case 'K': std::cout << rang::fgB::black; break;
            case 'R': std::cout << rang::fgB::red; break;
            case 'G': std::cout << rang::fgB::green; break;
            case 'Y': std::cout << rang::fgB::yellow; break;
            case 'B': std::cout << rang::fgB::blue; break;
            case 'M': std::cout << rang::fgB::magenta; break;
            case 'C': std::cout << rang::fgB::cyan; break;
            case 'W': std::cout << rang::fgB::gray; break;
            case '0':
                if (tag.size() >= 2) std::cout << rang::fg::reset;
                else std::cout << rang::style::reset;
                break;
            default: invalid_tag = true; break;
        }
        if (tag.size() >= 2) switch(tag[1])
        {
            case 'k': std::cout << rang::bg::black; break;
            case 'r': std::cout << rang::bg::red; break;
            case 'g': std::cout << rang::bg::green; break;
            case 'y': std::cout << rang::bg::yellow; break;
            case 'b': std::cout << rang::bg::blue; break;
            case 'm': std::cout << rang::bg::magenta; break;
            case 'c': std::cout << rang::bg::cyan; break;
            case 'w': std::cout << rang::bg::gray; break;
            case 'K': std::cout << rang::bgB::black; break;
            case 'R': std::cout << rang::bgB::red; break;
            case 'G': std::cout << rang::bgB::green; break;
            case 'Y': std::cout << rang::bgB::yellow; break;
            case 'B': std::cout << rang::bgB::blue; break;
            case 'M': std::cout << rang::bgB::magenta; break;
            case 'C': std::cout << rang::bgB::cyan; break;
            case 'W': std::cout << rang::bgB::gray; break;
            case '0': std::cout << rang::bg::reset; break;
            default: invalid_tag = true; break;
        }
        if (tag.size() > 2 || !tag.size()) invalid_tag = true;
        if (invalid_tag) invalid_tags.push_back(tag);
    }

    std::cout << rang::style::reset << '\n';    // Reset any ANSI tags and end the line.

    for (auto tag : invalid_tags)
        core().nonfatal("Invalid colour tag: {" + tag + "}", Core::CORE_WARN);
}

// Attempts to set the title of the console window. May not work on all platforms.
void set_window_title(const std::string& new_title)
{
#ifdef WESTGATE_TARGET_WINDOWS
    SetConsoleTitleA(new_title.c_str());
#else
    std::cout << "\033]2;" << new_title << "\007" << std::flush;
#endif
}

} } // namespace terminal, westgate
