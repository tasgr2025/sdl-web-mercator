#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cpr/cpr.h>

#include "tiletools.h"

#define IMG_INIT_EVERYTHING (IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF)

/// @brief Обработчик событий SDL
/// @param userdata Указатель на произвольные данные
/// @param event Событие 
/// @return 
int event_handler(void *userdata, SDL_Event *event);

#endif // MAIN_H
