// core/terminal.cpp -- Includes the rang library and iostream for printing coloured text to the terminal, as well as some basic utility functions.

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

#include <functional>
#include <iostream>

#if defined(WESTGATE_TARGET_LINUX) || defined(WESTGATE_TARGET_APPLE)
#include <cstdio>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#ifdef WESTGATE_TARGET_WINDOWS
#include <fstream>
#include <windows.h>
#endif

#include "3rdparty/rang/rang.hpp"
#include "core/core.hpp"
#include "core/terminal.hpp"

using std::cout;
using std::string;
using std::string_view;
using std::to_string;
using std::vector;

namespace westgate {
namespace terminal {

// Attempts to get the horizontal position of the 'cursor', where output is being printed. If anything goes wrong, it'll return 0.
unsigned int get_cursor_x()
{
#ifdef WESTGATE_TARGET_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) return 0;  // error
    return csbi.dwCursorPosition.X;
#else
    struct termios oldt{}, newt{};
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Request report
    std::printf("\033[6n");
    std::fflush(stdout);

    // Read answer
    int row, col;
    if (std::scanf("\033[%d;%dR", &row, &col) != 2) col = -1;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return (col > 0 ? col - 1 : 0);
#endif
}

// Prints a standard cursor and waits for non-zero input from the player.
const string get_input()
{
    cout << '\n' << rang::style::reset << rang::fgB::green << "> ";
    string input;
    do { std::getline(std::cin, input); } while(!input.size());
    cout << rang::style::reset;
    return input;
}

// As with get_input(), but requires the user to enter an integer number. If yes_no is true, it allows yes/no to translate to 1/0.
int get_number(int lowest, int highest, bool yes_no)
{
    int result = 0;
    string input;
    while(true)
    {
        input = get_input();
        if (!input.size()) continue;

        if (yes_no)
        {
            if (input[0] == 'y' || input[0] == 'Y') return 1;
            else if (input[0] == 'n' || input[0] == 'N') return 0;
        }

        try { result = std::stol(input); }
        catch(const std::invalid_argument&)
        {
            print("{Y}I'm sorry, that's not a valid number. Please only enter a number, without any symbols or letters.");
            continue;
        }
        catch(const std::out_of_range&)
        {
            print("{Y}That number is far too large, or invalid. Please try to be reasonable.");
            continue;
        }

        if (result < lowest || result > highest) print("{Y}Please enter a number between {R}" + to_string(lowest) + " {Y}and {R}" +
            to_string(highest) + ".");
        else return result;
    }
}

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
void print(const string_view text, bool newline)
{
    // Handle blank text by just printing a newline.
    if (!text.size())
    {
        if (newline) cout << '\n';
        return;
    }

    const unsigned int console_width = get_width();
    unsigned int chars_so_far = 0;

    // I KNOW THIS IS A NIGHTMARE OF CODE BUT THERE ARE SO MANY EDGE CASES :(
    std::function<void(string, bool)> print_formatted = [&print_formatted, &console_width, &chars_so_far](const string &line, bool recalc_cursor)
    {
        if (!line.size()) return;
        // This can be computationally expensive to do for every single word, especially on Unix, so we'll only check the 'cursor' position when absolutely
        // necessary, and keep count ourselves internally for the rest of it.
        if (recalc_cursor) chars_so_far = get_cursor_x();

        // If newlines are specified, break them up and handle them with recursive calls.
        if (auto new_line = line.find_first_of('\n');
            new_line != string::npos)
        {
            const string before_newline = line.substr(0, new_line);
            const string after_newline = line.substr(new_line + 1);
            print_formatted(before_newline, false);
            cout << '\n';
            chars_so_far = 0;
            print_formatted(after_newline, false);
            return;
        }

        // If there's any spaces, print one word at a time.
        auto space = line.find_first_of(' ');
        if (space == string::npos) // If there aren't any spaces, just print what we have as-is.
        {
            if (chars_so_far + line.size() >= console_width)
            {
                cout << '\n';
                chars_so_far = 0;
            }
            cout << line;
            chars_so_far += line.size();
            if (chars_so_far >= console_width) chars_so_far -= console_width;   // Make the best guess for how many characters ended up on the next line.
            return;
        }
        // Split the text into whatever's before the first space, and whatever's after it. This should let us print one word at a time.
        const string before_space = line.substr(0, space);
        const string after_space = line.substr(space + 1);
        bool space_after = false;
        if (line.at(line.size() - 1) == ' ' && !after_space.size()) space_after = true;

        // The current word is so long it can't be word-wrapped and is gonna split in half no matter what we do. Hnng. Okay, this is incredibly unlikely to
        // actually happen, but we've gotta handle it just in case.
        if (before_space.size() >= console_width)
        {
            int available_size = static_cast<int>(console_width) - chars_so_far;
            const string cropped_before = before_space.substr(0, available_size);
            cout << cropped_before << '\n';
            chars_so_far = 0;
            print_formatted(before_space.substr(available_size) + " " + after_space, false);
            return;
        }
        if (chars_so_far + before_space.size() > console_width) // If there isn't room for this word, start a new line.
        {
            cout << '\n';
            chars_so_far = 0;
        }
        if (after_space.size()) space_after = true;
        cout << before_space;
        chars_so_far += before_space.size();
        if (chars_so_far == console_width)  // If the word *ends* at the very last column of the console window, insert a new line.
        {
            cout << '\n';
            chars_so_far = 0;
        }
        else if (space_after)
        {
            cout << ' ';
            chars_so_far++;
        }
        print_formatted(after_space, false);
    };

    string output = string{text};
    vector<string> invalid_tags;
    while(output.size())
    {
        auto opener_pos = output.find_first_of('{');    // Try to find an opening tag for an ANSI colour.
        auto closer_pos = output.find_first_of('}');    // And a matching closing tag.
        if (opener_pos == string::npos || closer_pos == string::npos) // If none are found, just print the string as-is and finish.
        {
            print_formatted(output, true);
            break;
        }

        // Clip out the text before, during, and after the colour tag.
        string untagged = output.substr(0, opener_pos);
        string tag = output.substr(opener_pos + 1, closer_pos - opener_pos - 1);
        output = output.substr(closer_pos + 1);

        // Print any text before the next ANSI tag. If there's an existing tag already in play, it'll cause this text to be coloured.
        if (untagged.size()) print_formatted(untagged, false);

        // Process the ANSI codes from the tag.
        bool invalid_tag = false;
        if (tag.size() >= 1) switch(tag[0])
        {
            case 'k': cout << rang::fg::black; break;
            case 'r': cout << rang::fg::red; break;
            case 'g': cout << rang::fg::green; break;
            case 'y': cout << rang::fg::yellow; break;
            case 'b': cout << rang::fg::blue; break;
            case 'm': cout << rang::fg::magenta; break;
            case 'c': cout << rang::fg::cyan; break;
            case 'w': cout << rang::fg::gray; break;
            case 'K': cout << rang::fgB::black; break;
            case 'R': cout << rang::fgB::red; break;
            case 'G': cout << rang::fgB::green; break;
            case 'Y': cout << rang::fgB::yellow; break;
            case 'B': cout << rang::fgB::blue; break;
            case 'M': cout << rang::fgB::magenta; break;
            case 'C': cout << rang::fgB::cyan; break;
            case 'W': cout << rang::fgB::gray; break;
            case '0':
                if (tag.size() >= 2) cout << rang::fg::reset;
                else cout << rang::style::reset;
                break;
            default: invalid_tag = true; break;
        }
        if (tag.size() >= 2) switch(tag[1])
        {
            case 'k': cout << rang::bg::black; break;
            case 'r': cout << rang::bg::red; break;
            case 'g': cout << rang::bg::green; break;
            case 'y': cout << rang::bg::yellow; break;
            case 'b': cout << rang::bg::blue; break;
            case 'm': cout << rang::bg::magenta; break;
            case 'c': cout << rang::bg::cyan; break;
            case 'w': cout << rang::bg::gray; break;
            case 'K': cout << rang::bgB::black; break;
            case 'R': cout << rang::bgB::red; break;
            case 'G': cout << rang::bgB::green; break;
            case 'Y': cout << rang::bgB::yellow; break;
            case 'B': cout << rang::bgB::blue; break;
            case 'M': cout << rang::bgB::magenta; break;
            case 'C': cout << rang::bgB::cyan; break;
            case 'W': cout << rang::bgB::gray; break;
            case '0': cout << rang::bg::reset; break;
            default: invalid_tag = true; break;
        }
        if (tag.size() > 2 || !tag.size()) invalid_tag = true;
        if (!tag.compare("nl")) invalid_tag = false;
        if (invalid_tag) invalid_tags.push_back(tag);
    }

    if (newline) cout << rang::style::reset << '\n';   // Reset any ANSI tags and end the line, if requested.

    for (auto tag : invalid_tags)
        core().nonfatal("Invalid colour tag: {" + tag + "}", Core::CORE_WARN);
}

// Attempts to set the title of the console window. May not work on all platforms.
void set_window_title(const string_view new_title)
{
#ifdef WESTGATE_TARGET_WINDOWS
    SetConsoleTitleA(string{new_title}.c_str());
#else
    cout << "\033]2;" << new_title << "\007" << std::flush;
#endif
}

} } // namespace terminal, westgate
