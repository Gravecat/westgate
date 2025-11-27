// world/time/time-weather.hpp -- The time and weather system.
// Weather system originally based on Keran's MUSH/MUX Weather and Time Code Package Version 4.0 beta, copyright (c) 1996-1998 Keran (keranset@mail1.nai.net).
// https://www.mushcode.com/File/Kerans-Weather-System-And-Time-Code-4-0-(PennMUSH)

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2021, 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "core/pch.hpp"

#include <map>

namespace trailmix
{
class FileReader;   // defined in trailmix/file/filereader.hpp
class FileWriter;   // defined in trailmix/file/filewriter.hpp
}

namespace westgate {

class TimeWeather
{
public:
    enum class Indoors : uint8_t { INDOORS, INDOORS_NO_WINDOWS, INDOORS_CITY, OUTSIDE, OUTSIDE_CITY, UNDERGROUND };
    enum class LightDark : uint8_t { LIGHT, DARK, NIGHT };
    enum class LunarPhase : uint8_t { NEW, WAXING_CRESCENT, FIRST_QUARTER, WAXING_GIBBOUS, FULL, WANING_GIBBOUS, THIRD_QUARTER, WANING_CRESCENT };
    enum class Season : uint8_t { AUTO, WINTER, SPRING, SUMMER, AUTUMN };
    enum Time { SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400 };
    enum class TimeOfDay : uint8_t { DAWN, SUNRISE, MORNING, NOON, SUNSET, DUSK, NIGHT, MIDNIGHT, DAY };
    enum class Weather : uint8_t { BLIZZARD, STORMY, RAIN, CLEAR, FAIR, OVERCAST, FOG, LIGHTSNOW, SLEET };

                TimeWeather();              // Sets up the time and weather system with default values, and loads its strings into memory.
    Season      current_season();           // Gets the current season.
    std::string day_name();                 // Returns the name of the current day of the week.
    int         day_of_month();             // Returns the current day of the month.
    std::string day_of_month_string();      // Returns the day of the month in the form of a string like "1st" or "19th".
    Indoors     indoors();                  // Gets the indoors/outdoors state.
    LightDark   light_dark();               // Checks whether it's light or dark right now.
    void        load_data(trailmix::FileReader* file);  // Loads the time/weather data from the specified save file.
    std::string month_name();               // Returns the name of the current month.
    LunarPhase  moon_phase();               // Gets the current lunar phase.
    bool        pass_time(float seconds, bool allow_interrupt = false); // Causes time to pass.
    Season      room_season();              // Retrieves the season override (if any) for the current Room.
    void        save_data(trailmix::FileWriter* file);  // Saves the time/weather data to the specified save file.
    std::string season_str(Season season);  // Converts a season integer to a string.
    void        tick();                     // Advances time by the smallest possible gradient; useful for loops waiting for something to happen.
    TimeOfDay   time_of_day(bool fine);     // Returns the current time of day (morning, day, dusk, night)
    int         time_of_day_exact();        // Returns the exact time of day.
    std::string time_of_day_str(bool fine); // Returns the current time of day as a string.
    uint64_t    time_passed();              // Returns the total amount of time passed in this game.
    Weather     weather();                  // Gets the current weather, runs fix_weather() internally.
    std::string weather_desc();             // Returns a weather description for the current time/weather, based on the current season.
    std::string weather_str(Weather weather);   // Converts a weather integer to a string.

private:
    static constexpr int    LUNAR_CYCLE_DAYS =  29;     // How many days are in a lunar cycle?
    static constexpr float  TIME_GRANULARITY =  0.1f;   // The lower this number, the more fine-grained the accuracy of the passage of time becomes.
    static constexpr int    TIME_WEATHER_SAVE_VERSION = 1;  // The version of the time/weather saved data in the saved game file.

    Weather     fix_weather(Weather weather, Season season);    // Fixes weather for a specified season.
    void        trigger_event(std::string *message_to_append, bool silent); // Triggers a time-change event.
    bool        player_near_trees();                            // Is the player near trees right now?
    void        replace_tokens(std::string &str, Indoors indoor_state); // Replaces tokens like $LANDSCAPE|STREETS$ in the source message with correct text.
    std::string weather_desc(Season season, bool trees);        // Returns a weather description for the current time/weather, based on the specified season.

    int         day_;       // The current day of the year.
    int         moon_;      // The current moon phase.
    int         time_;      // The time of day.
    uint64_t    time_passed_;   // The total amount of time that has passed in this game.
    float       time_passed_subsecond_; // For counting time passed in amounts of time less than a second.
    Weather     weather_;   // The current weather.

    std::map<std::string, std::string>  tw_string_map_; // The time and weather transition strings.
    std::vector<std::string>            weather_change_map_;    // Weather change maps, to determine odds of changing to different weather types.
};

}   // namespace westgate
