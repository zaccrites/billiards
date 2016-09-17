
#include <iostream>

#include "Game.hpp"
#include "debug.hpp"

// Todo: Create makefile
// g++ -std=c++14 -Wall -Wextra -Weffc++ -Wshadow -Wcast-qual -Wold-style-cast -Wfloat-equal -isystem include -O2 -DNDEBUG src/*.cpp -lSDL2 -o billiards

//    on OSX add:  -Wl,-framework,Cocoa


// int main(int argc, char** argv)
int main()
{
    DEBUG_LOG("Process start \n");

    Game game;

    DEBUG_LOG("Setting up SDL \n");
    if ( ! game.setupSDL())
    {
        std::cerr
            << "Could not initialize SDL! \n  "
            << SDL_GetError()
            << std::endl;
        return 1;
    }

    DEBUG_LOG("Starting game \n");
    if ( ! game.start())
    {
        std::cerr
            << "Error starting game!"
            << std::endl;
        return 1;
    }

    return 0;
}
