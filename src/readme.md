# src folder details

## [3rdparty](3rdparty)
Third-party code included in full in this project. Some of this stuff is header-only, some is full-on code that's compiled separately in CMake and linked into
the project.

There are some very minor modifications here and there, but for the most part, this is pretty much stock code.

##  [actions](actions)
Code to handle player commands in-game, generally called from the parser. A bulk of the player's interactions with the game are coded here.

## [cmake](cmake)
Various small header files that are modified with CMake to make things like the build number available to the C++ source code.

## [core](core)
The central core parts of the game engine:

`Core` handles initialization, shutdown, and error handling/logging.

`Game` handles the main game loop, new-game, load-game and the title screen.

`Terminal` interacts with both the terminal (or terminal window) at a system level, and provides a layer of abstraction for using the rang library.

`pch.hpp` is a precompiled global header that includes various extremely-frequently-used system libraries like `<string>`.

## [parser](parser)
The parser that takes input from the player and translates it into in-game commands, usually by calling code in [actions](actions).

## [util](util)
Utility functions, some extremely generic, some specialized for this project. Should be pretty obvious what does what and why.

## [world](world)
Class definitions for various parts of the game world.

[world/area](world/area) contains `Room` (a location in the game world), `Region` (a collection of Rooms), `Link` (a connection between Rooms), and the automap
code.

[world/entity](world/entity) contains `Entity` and its derived classes, `Item` (things that can be picked up and used), `Mobile` (things that move around in the
game world) and `Player` (a type of Mobile specialized for the player character). `Inventory` is also included here, a management class that handles collections
of Items being contained in one place.

[world/time](world/time) contains the timing systems and the time/weather handler.

Finally, `World` is an overall world manager class that ties all these systems together and handles saving/loading of world data.
