#include "sdl_driver.h"

void SDL_Init_Driver()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        printf("[*] Failed to initialise SDL audio subsystem: %s\n", SDL_GetError());
}
