
#include <iostream>

#include "Game.hpp"
#include "debug.hpp"


int main()
{
    DEBUG_LOG("Process start \n");

    Game game;

    if ( ! game.setupSDL())
    {
        std::cerr
            << "Could not initialize SDL! \n  "
            << SDL_GetError()
            << std::endl;
        return 1;
    }

    if ( ! game.start())
    {
        std::cerr
            << "Error starting game!"
            << std::endl;
        return 1;
    }

    return 0;
}
