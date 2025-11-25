// world/time-weather.hpp -- The time and weather system.
// Weather system originally based on Keran's MUSH/MUX Weather and Time Code Package Version 4.0 beta, copyright (c) 1996-1998 Keran (keranset@mail1.nai.net).
// https://www.mushcode.com/File/Kerans-Weather-System-And-Time-Code-4-0-(PennMUSH)

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2021, 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "util/file/yaml.hpp"
#include "util/math/random.hpp"
#include "util/text/stringutils.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"
#include "world/time-weather.hpp"

#include <cmath>
#include <filesystem>

using std::floor;
using std::runtime_error;
using std::string;
using std::to_string;
using std::vector;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Sets up the time and weather system with default values, and loads its strings into memory.
TimeWeather::TimeWeather() : day_(80), moon_(1), time_(39660), time_passed_(0), time_passed_subsecond_(0), weather_(Weather::FAIR)
{
    weather_change_map_.resize(9);

    const std::string filename = core().datafile("misc/weather.yml");
    if (!fs::is_regular_file(filename)) throw runtime_error("Could not load weather.yml!");
    YAML yaml(filename);
    if (!yaml.is_map()) throw runtime_error("weather.yml file is invalid!");
    auto key_vals = yaml.keys_vals();
    for (auto &key_val : key_vals)
    {
        const std::string key = key_val.first;
        const std::string val = key_val.second;

        if (key.size() == 5 && !key.substr(0,4).compare("WMAP"))
        {
            const int map_id = key.at(4) - '0';
            if (map_id < 0 || map_id > 8) throw runtime_error("Invalid weather map strings.");
            weather_change_map_.at(map_id) = stringutils::decode_compressed_string(val);
        }
        else tw_string_map_.insert({key, val});
    }
}

// Gets the current season.
TimeWeather::Season TimeWeather::current_season()
{
    // Rooms can override the season with a fixed value, useful for deserts, icy mountains, etc.
    Room* room = player().parent_room();
    if (room->tag(RoomTag::AlwaysWinter)) return Season::WINTER;
    if (room->tag(RoomTag::AlwaysSpring)) return Season::SPRING;
    if (room->tag(RoomTag::AlwaysSummer)) return Season::SUMMER;
    if (room->tag(RoomTag::AlwaysAutumn)) return Season::AUTUMN;

    if (day_ > 364) throw runtime_error("Impossible day specified!");
    if (day_ < 79) return Season::WINTER;
    else if (day_ < 172) return Season::SPRING;
    else if (day_ <= 266) return Season::SUMMER;
    else if (day_ <= 355) return Season::AUTUMN;
    else return Season::WINTER;
}

// Returns the name of the current day of the week.
string TimeWeather::day_name()
{
    unsigned int temp_day = day_;
    while (temp_day > 7) temp_day -= 7;
    switch (temp_day)
    {
        case 1: return "Sunsday";       // Sunday
        case 2: return "Moonsday";      // Monday
        case 3: return "Heavensday";    // Tuesday
        case 4: return "Oathsday";      // Wednesday
        case 5: return "Crownsday";     // Thursday
        case 6: return "Swordsday";     // Friday
        case 7: return "Silversday";    // Saturday
    }
    return "";  // Just to make the compiler shut up.
}

// Returns the current day of the month.
int TimeWeather::day_of_month()
{
    if (day_ <= 28) return day_;
    unsigned int temp_day = day_;
    while (temp_day > 28) temp_day -= 28;
    return temp_day;
}

// Returns the day of the month in the form of a string like "1st" or "19th".
string TimeWeather::day_of_month_string()
{
    const int dom = day_of_month();
    string str = to_string(dom);
    if (dom == 1 || dom == 21) return str + "st";
    else if (dom == 2 || dom == 22) return str += "nd";
    else if (dom == 3 || dom == 23) return str += "rd";
    return str + "th";
}

// Fixes weather for a specified season.
TimeWeather::Weather TimeWeather::fix_weather(TimeWeather::Weather weather, TimeWeather::Season season)
{
    if (season == Season::SPRING && weather == Weather::SLEET) weather = Weather::RAIN;
    else if (season == Season::SUMMER || season == Season::AUTUMN)
    {
        if (weather == Weather::BLIZZARD) weather = Weather::STORMY;
        else if (weather == Weather::LIGHTSNOW || weather == Weather::SLEET) weather = Weather::RAIN;
    }
    return weather;
}

// Gets the indoors/outdoors state.
TimeWeather::Indoors TimeWeather::indoors()
{
    const Room* room = player().parent_room();
    if (room->tag(RoomTag::Underground)) return Indoors::UNDERGROUND;
    if (room->tag(RoomTag::Indoors))
    {
        if (room->tag(RoomTag::Windows)) return Indoors::INDOORS;
        else return Indoors::INDOORS_NO_WINDOWS;
    }
    if (room->tag(RoomTag::Streets)) return Indoors::OUTSIDE_STREETS;
    return Indoors::OUTSIDE;
}

// Checks whether it's light or dark right now.
TimeWeather::LightDark TimeWeather::light_dark()
{
    if (time_ >= 1285 * Time::MINUTE) return LightDark::NIGHT;
    else if (time_ >= 1140 * Time::MINUTE) return LightDark::DARK;
    else if (time_ >= 420 * Time::MINUTE) return LightDark::LIGHT;
    else if (time_ >= 277 * Time::MINUTE) return LightDark::DARK;
    else return LightDark::NIGHT;
}

// Returns the name of the current month.
string TimeWeather::month_name()
{
    if (day_ <= 28) return "Harrowing";             // January
    else if (day_ <= 56) return "Shadows";          // February
    else if (day_ <= 84) return "the Lord";         // March
    else if (day_ <= 112) return "the Lady";        // April
    else if (day_ <= 140) return "the Fall";        // May
    else if (day_ <= 168) return "Fortune";         // June
    else if (day_ <= 196) return "Fire";            // Sol
    else if (day_ <= 224) return "Gold";            // July
    else if (day_ <= 252) return "Seeking";         // August
    else if (day_ <= 280) return "the Serpent";     // September
    else if (day_ <= 308) return "Crimson";         // October
    else if (day_ <= 336) return "King's Night";    // November
    else return "Frost";                            // December
}

// Gets the current lunar phase.
TimeWeather::LunarPhase TimeWeather::moon_phase()
{
    switch (moon_)
    {
        case 0: return LunarPhase::NEW;
        case 1: case 2: case 3: case 4: case 5: case 6: return LunarPhase::WAXING_CRESCENT;
        case 7: case 8: case 9: return LunarPhase::FIRST_QUARTER;
        case 10: case 11: case 12: case 13: case 14: return LunarPhase::WAXING_GIBBOUS;
        case 15: return LunarPhase::FULL;
        case 16: case 17: case 18: case 19: case 20: return LunarPhase::WANING_GIBBOUS;
        case 21: case 22: case 23: return LunarPhase::THIRD_QUARTER;
        case 24: case 25: case 26: case 27: case 28: return LunarPhase::WANING_CRESCENT;
        default: throw runtime_error("Impossible moon phase!");
    }
}

// Causes time to pass.
bool TimeWeather::pass_time(float seconds, bool allow_interrupt)
{
    // Add to the total time passed count.
    time_passed_subsecond_ += seconds;
    int seconds_to_add = 0;
    if (time_passed_subsecond_ >= 1.0f)
    {
        seconds_to_add = floor(time_passed_subsecond_);
        time_passed_ += seconds_to_add;
        time_passed_subsecond_ -= seconds_to_add;
    }

    //int player_old_hp = World::player()->hp();
    while (seconds_to_add)
    {
        seconds_to_add--;
        //if (World::player()->game_over()) return false;
        if (allow_interrupt)
        {
            //if (World::player()->hp() < player_old_hp) return false;
            //player_old_hp = World::player()->hp();  // Because HP might go *up* in the meantime.
        }
        Indoors indoor_state = indoors();
        const bool show_weather_messages = (indoor_state == Indoors::OUTSIDE || indoor_state == Indoors::OUTSIDE_STREETS || indoor_state == Indoors::INDOORS);

        TimeOfDay old_time_of_day = time_of_day(true);
        int old_time = time_;
        bool change_happened = false;
        string weather_msg;
        if (++time_ >= Time::DAY) time_ -= Time::DAY;
        if (time_ >= 420 * Time::MINUTE && old_time < 420 * Time::MINUTE)   // Trigger moon-phase changing and day-of-year changing at dawn, not midnight.
        {
            if (++day_ > 364) day_ = 1;
            if (++moon_ >= LUNAR_CYCLE_DAYS) moon_ = 0;
            print("{y}It is now " + day_name() + ", the " + day_of_month_string() + " day of " +  month_name() + ".");
        }
        old_time = time_;
        if (time_of_day(true) != old_time_of_day)
        {
            weather_msg = "";
            old_time_of_day = time_of_day(true);
            trigger_event(&weather_msg, !show_weather_messages);
            change_happened = show_weather_messages;
        }

        if (change_happened) print(weather_msg.substr(1));

        //Encounters::tick(1);

        // Ticks all currently-active Mobiles.
        /*
        for (auto mob : *World::mobiles())
            mob->tick(1);
        */
    }

    return true;
}

// Is the player near trees right now?
bool TimeWeather::player_near_trees()
{ return player().parent_room()->tag(RoomTag::Trees); }

// Converts a season integer to a string.
string TimeWeather::season_str(TimeWeather::Season season)
{
    switch (season)
    {
        case Season::WINTER: return "WINTER";
        case Season::SPRING: return "SPRING";
        case Season::AUTUMN: return "AUTUMN";
        case Season::SUMMER: return "SUMMER";
        default: throw runtime_error("Invalid season specified!");
    }
    return "";
}

// Returns the current time of day (morning, day, dusk, night)
TimeWeather::TimeOfDay TimeWeather::time_of_day(bool fine)
{
    if (fine)
    {
        if (time_ >= 1380 * Time::MINUTE) return TimeOfDay::MIDNIGHT;
        else if (time_ >= 1260 * Time::MINUTE) return TimeOfDay::NIGHT;
        else if (time_ >= 1140 * Time::MINUTE) return TimeOfDay::DUSK;
        else if (time_ >= 1020 * Time::MINUTE) return TimeOfDay::SUNSET;
        else if (time_ >= 660 * Time::MINUTE) return TimeOfDay::NOON;
        else if (time_ >= 540 * Time::MINUTE) return TimeOfDay::MORNING;
        else if (time_ >= 420 * Time::MINUTE) return TimeOfDay::SUNRISE;
        else if (time_ >= 300 * Time::MINUTE) return TimeOfDay::DAWN;
        return TimeOfDay::MIDNIGHT;
    } else
    {
        if (time_ >= 1380 * Time::MINUTE) return TimeOfDay::NIGHT;
        if (time_ >= 1140 * Time::MINUTE) return TimeOfDay::DUSK;
        if (time_ >= 540 * Time::MINUTE) return TimeOfDay::DAY;
        if (time_ >= 300 * Time::MINUTE) return TimeOfDay::DAWN;
        return TimeOfDay::NIGHT;
    }
}

// Returns the exact time of day.
int TimeWeather::time_of_day_exact() { return time_; }

// Returns the current time of day as a string.
string TimeWeather::time_of_day_str(bool fine)
{
    if (fine)
    {
        if (time_ >= 1380 * Time::MINUTE) return "MIDNIGHT";
        else if (time_ >= 1260 * Time::MINUTE) return "NIGHT";
        else if (time_ >= 1140 * Time::MINUTE) return "DUSK";
        else if (time_ >= 1020 * Time::MINUTE) return "SUNSET";
        else if (time_ >= 660 * Time::MINUTE) return "NOON";
        else if (time_ >= 540 * Time::MINUTE) return "MORNING";
        else if (time_ >= 420 * Time::MINUTE) return "SUNRISE";
        else if (time_ >= 300 * Time::MINUTE) return "DAWN";
        return "NIGHT";
    } else
    {
        if (time_ >= 1380 * Time::MINUTE) return "NIGHT";
        if (time_ >= 1140 * Time::MINUTE) return "DUSK";
        if (time_ >= 540 * Time::MINUTE) return "DAY";
        if (time_ >= 300 * Time::MINUTE) return "DAWN";
        return "NIGHT";
    }
}

// Advances time by the smallest possible gradient; useful for loops waiting for something to happen.
void TimeWeather::tick() { pass_time(TIME_GRANULARITY); }

// Returns the total amount of time passed in this game.
uint64_t TimeWeather::time_passed() { return time_passed_; }

void TimeWeather::trigger_event(string *message_to_append, bool silent)
{
    const string weather_map = weather_change_map_.at(static_cast<int>(weather_));
    const char new_weather = weather_map[random::get<int>(0, weather_map.size() - 1)];
    switch (new_weather)
    {
        case 'c': weather_ = Weather::CLEAR; break;
        case 'f': weather_ = Weather::FAIR; break;
        case 'r': weather_ = Weather::RAIN; break;
        case 'F': weather_ = Weather::FOG; break;
        case 'S': weather_ = Weather::STORMY; break;
        case 'o': weather_ = Weather::OVERCAST; break;
        case 'b': weather_ = Weather::BLIZZARD; break;
        case 'l': weather_ = Weather::LIGHTSNOW; break;
        case 'L': weather_ = Weather::SLEET; break;
    }
    if (silent) return;

    // Display an appropriate message for the changing time/weather, if we're outdoors.
    const string time_message = tw_string_map_.at(time_of_day_str(true) + "_" + weather_str(fix_weather(weather_, current_season())) +
        (indoors() == Indoors::INDOORS ? "_INDOORS" : ""));
    if (message_to_append) *message_to_append += " " + time_message;
    else print(time_message);
}

// Gets the current weather, runs fix_weather() internally.
TimeWeather::Weather TimeWeather::weather()
{ return fix_weather(weather_, current_season()); }

// Returns a weather description for the current time/weather, based on the current season.
string TimeWeather::weather_desc()
{ return weather_desc(current_season(), player_near_trees()); }

// Returns a weather description for the current time/weather, based on the specified season.
string TimeWeather::weather_desc(TimeWeather::Season season, bool trees)
{
    const Indoors indoor_state = indoors();
    const bool indoor_bool = (indoor_state == Indoors::INDOORS || indoor_state == Indoors::INDOORS_NO_WINDOWS);
    const Weather weather = fix_weather(weather_, season);
    string desc = tw_string_map_.at(season_str(season) + "_" + time_of_day_str(false) + "_" + weather_str(weather)  + (indoor_bool ? "_INDOORS" : ""));
    if (trees)
    {
        string tree_time = "DAY";
        if (time_of_day(false) == TimeOfDay::DUSK || time_of_day(false) == TimeOfDay::NIGHT) tree_time = "NIGHT";
        desc += " " + tw_string_map_.at(season_str(season) + "_" + tree_time + "_" + weather_str(weather) + "_TREES");
    }
    if (indoor_state == Indoors::OUTSIDE_STREETS) stringutils::find_and_replace(desc, "landscape", "city");
    return desc;
}

// Converts a weather integer to a string.
string TimeWeather::weather_str(TimeWeather::Weather weather)
{
    switch (weather)
    {
        case Weather::BLIZZARD: return "BLIZZARD";
        case Weather::STORMY: return "STORMY";
        case Weather::RAIN: return "RAIN";
        case Weather::CLEAR: return "CLEAR";
        case Weather::FAIR: return "FAIR";
        case Weather::OVERCAST: return "OVERCAST";
        case Weather::FOG: return "FOG";
        case Weather::LIGHTSNOW: return "LIGHTSNOW";
        case Weather::SLEET: return "SLEET";
        default: throw runtime_error("Invalid weather specified: " + to_string(static_cast<int>(weather)));
    }
    return "";
}

}   // namespace westgate
