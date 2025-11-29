// world/time/time-weather.hpp -- The time and weather system.
// Weather system originally based on Keran's MUSH/MUX Weather and Time Code Package Version 4.0 beta, copyright (c) 1996-1998 Keran (keranset@mail1.nai.net).
// https://www.mushcode.com/File/Kerans-Weather-System-And-Time-Code-4-0-(PennMUSH)

// SPDX-FileType: SOURCE
// SPDX-FileCopyrightText: Copyright 2021, 2025 Raine Simmons <gc@gravecat.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/core.hpp"
#include "core/terminal.hpp"
#include "trailmix/file/filereader.hpp"
#include "trailmix/file/filewriter.hpp"
#include "trailmix/file/yaml.hpp"
#include "trailmix/math/random.hpp"
#include "trailmix/text/formatting.hpp"
#include "trailmix/text/manipulation.hpp"
#include "world/area/link.hpp"
#include "world/area/room.hpp"
#include "world/entity/player.hpp"
#include "world/time/time-weather.hpp"

#include <cmath>
#include <filesystem>

using std::runtime_error;
using std::string;
using std::to_string;
using std::vector;
using trailmix::file::FileReader;
using trailmix::file::FileWriter;
using trailmix::file::YAML;
using trailmix::math::rnd;
using trailmix::text::formatting::process_conditional_tags;
using trailmix::text::manipulation::decode_compressed_string;
using trailmix::text::manipulation::find_and_replace;
using westgate::terminal::print;
namespace fs = std::filesystem;

namespace westgate {

// Sets up the time and weather system with default values, and loads its strings into memory.
TimeWeather::TimeWeather() : time_passed_(0), time_passed_subsecond_(0)
{
    // Slightly randomize the starting time, but keep it within certain parameters (early- to mid-spring, between sunrise and noon).
    day_ = rnd::get<int>(80, 130);
    moon_ = (day_ - 79) % LUNAR_CYCLE_DAYS;
    time_ = rnd::get<int>(420 * Time::MINUTE, 660 * Time::MINUTE);
    wind_clockwise_ = rnd::get<bool>(0.5f);
    wind_direction_ = static_cast<Direction>(rnd::get<int>(1, 8));
    wind_next_change_ = rnd::get<int>(2 * HOUR, 4 * HOUR);

    // The starting weather is always either clear or fair.
    if (rnd::get<bool>(0.5f)) weather_ = Weather::CLEAR;
    else weather_ = Weather::FAIR;

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
            weather_change_map_.at(map_id) = decode_compressed_string(val);
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

// Checks whether it's light or dark right now.
TimeWeather::LightDark TimeWeather::light_dark()
{
    if (time_ >= 1285 * Time::MINUTE) return LightDark::NIGHT;
    else if (time_ >= 1140 * Time::MINUTE) return LightDark::DARK;
    else if (time_ >= 420 * Time::MINUTE) return LightDark::LIGHT;
    else if (time_ >= 277 * Time::MINUTE) return LightDark::DARK;
    else return LightDark::NIGHT;
}

// Loads the time/weather data from the specified save file.
void TimeWeather::load_data(FileReader* file)
{
    const uint32_t tw_save_ver = file->read_data<uint32_t>();
    if (tw_save_ver != TIME_WEATHER_SAVE_VERSION) FileReader::standard_error("Incompatible time/weather data version", tw_save_ver, TIME_WEATHER_SAVE_VERSION);
    day_ = file->read_data<int>();
    moon_ = file->read_data<int>();
    time_ = file->read_data<int>();
    time_passed_ = file->read_data<uint64_t>();
    time_passed_subsecond_ = file->read_data<float>();
    weather_ = file->read_data<Weather>();
    wind_clockwise_ = file->read_data<bool>();
    wind_direction_ = file->read_data<Direction>();
    wind_next_change_ = file->read_data<uint64_t>();
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
        const bool can_see_outside = player().parent_room()->can_see_outside();

        // Check if it's due time for the wind to change direction. First, we'll shorten the duration if there's a storm ongoing.
        const bool storm = (weather_ == Weather::BLIZZARD || weather_ == Weather::STORMY);
        if (storm && wind_next_change_ > time_passed_ - seconds_to_add)
        {
            const uint64_t wind_change_time_left = wind_next_change_ - time_passed_ - seconds_to_add;
            if (wind_change_time_left > HOUR) wind_next_change_ = time_passed_ - seconds_to_add + rnd::get<int>(30 * MINUTE, 60 * MINUTE);
        }
        // If it's due to change direction now, let's do it.
        if (time_passed_ - seconds_to_add > wind_next_change_)
        {
            if (storm) wind_next_change_ = time_passed_ - seconds_to_add + rnd::get<int>(30 * MINUTE, 60 * MINUTE);
            else wind_next_change_ = time_passed_ - seconds_to_add + rnd::get<int>(2 * HOUR, 4 * HOUR);
            if (rnd::get<bool>(storm ? 0.5f : 0.1f))    // 10% chance (60% during storms) of the wind randomizing its direction and rotation
            {
                wind_clockwise_ = rnd::get<bool>(0.5f);
                wind_direction_ = static_cast<Direction>(rnd::get<int>(1, 8));
            }
            // 35% chance (80% during storms) for the wind's rotation to switch.
            else if (rnd::get<bool>(storm ? 0.8f : 0.35f)) wind_clockwise_ = !wind_clockwise_;

            // Rotate the wind, and apply the new value.
            uint8_t wind_dir_int = static_cast<uint8_t>(wind_direction_);
            wind_dir_int = ((wind_dir_int - 1 + (wind_clockwise_ ? 1 : -1) + 8) % 8) + 1;
            wind_direction_ = static_cast<Direction>(wind_dir_int);
        }

        TimeOfDay old_time_of_day = time_of_day(true);
        int old_time = time_;
        bool change_happened = false;
        string weather_msg;
        if (++time_ >= Time::DAY) time_ -= Time::DAY;
        if (time_ >= 420 * Time::MINUTE && old_time < 420 * Time::MINUTE)   // Trigger moon-phase changing and day-of-year changing at dawn, not midnight.
        {
            if (++day_ > 364) day_ = 1;
            if (++moon_ >= LUNAR_CYCLE_DAYS) moon_ = 0;
            print("{Y}It is now " + day_name() + ", the " + day_of_month_string() + " day of " +  month_name() + ".");
        }
        old_time = time_;
        if (time_of_day(true) != old_time_of_day)
        {
            weather_msg = "";
            old_time_of_day = time_of_day(true);
            trigger_event(&weather_msg, !can_see_outside);
            change_happened = can_see_outside;
        }

        if (change_happened) print("{y}" + weather_msg.substr(1));

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
}

// Saves the time/weather data to the specified datafile.
void TimeWeather::save_data(FileWriter* file)
{
    file->write_data<uint32_t>(TIME_WEATHER_SAVE_VERSION);
    file->write_data<int>(day_);
    file->write_data<int>(moon_);
    file->write_data<int>(time_);
    file->write_data<uint64_t>(time_passed_);
    file->write_data<float>(time_passed_subsecond_);
    file->write_data<Weather>(weather_);
    file->write_data<bool>(wind_clockwise_);
    file->write_data<Direction>(wind_direction_);
    file->write_data<uint64_t>(wind_next_change_);
}

// Retrieves a message directly from the string map, with tags processed.
std::string TimeWeather::string_map(const std::string& key)
{
    const Room* player_room = player().parent_room();
    const bool indoors = player_room->tag(RoomTag::Indoors) || player_room->tag(RoomTag::Underground);
    const bool in_city = player_room->tag(RoomTag::City);
    auto result = tw_string_map_.find(key);
    if (result == tw_string_map_.end())
    {
        core().nonfatal("Unable to retrieve time/weather string: " + key, Core::CORE_ERROR);
        return "";
    }
    std::string out = result->second;
    process_conditional_tags(out, "outside", !indoors);
    process_conditional_tags(out, "inside", indoors);
    if (in_city)
    {
        find_and_replace(out, "$GROUND|STREET$", "street");
        find_and_replace(out, "$LAND|CITY$", "city");
        find_and_replace(out, "$LAND|STREET$", "street");
        find_and_replace(out, "$LAND|STREETS$", "streets");
        find_and_replace(out, "$LANDSCAPE|CITY$", "city");
        find_and_replace(out, "$LANDSCAPE|STREETS$", "streets");
    }
    else
    {
        find_and_replace(out, "$GROUND|STREET$", "ground");
        find_and_replace(out, "$LAND|CITY$", "land");
        find_and_replace(out, "$LAND|STREET$", "land");
        find_and_replace(out, "$LAND|STREETS$", "land");
        find_and_replace(out, "$LANDSCAPE|CITY$", "landscape");
        find_and_replace(out, "$LANDSCAPE|STREETS$", "landscape");
    }
    find_and_replace(out, "$WIND_DIR$", Room::direction_name(wind_direction_));
    return out;
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
    const char new_weather = weather_map[rnd::get<int>(0, weather_map.size() - 1)];
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

    // Display an appropriate message for the changing time/weather.
    string time_message = string_map(time_of_day_str(true) + "_" + weather_str(fix_weather(weather_, current_season())));
    if (message_to_append) *message_to_append += " " + time_message;
    else print("{y}" + time_message);
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
    const Weather weather = fix_weather(weather_, season);
    string desc = string_map(season_str(season) + "_" + time_of_day_str(false) + "_" + weather_str(weather));
    if (trees)
    {
        string tree_time = "DAY";
        if (time_of_day(false) == TimeOfDay::DUSK || time_of_day(false) == TimeOfDay::NIGHT) tree_time = "NIGHT";
        desc += " " + string_map(season_str(season) + "_" + tree_time + "_" + weather_str(weather) + "_TREES");
    }
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
}

}   // namespace westgate
