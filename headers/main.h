#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cpr/cpr.h>
#include <CLI.hpp>
#include <cpptrace/from_current.hpp>

#include "sdltile.h"
#include "tiletools.h"

#define exit_on_sdl_error() { printf("%s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); cpptrace::from_current_exception().print(); throw std::runtime_error(""); }
#define warn_on_sdl_error() { printf("внимание: %s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); }

/// @brief Обработчик событий SDL
/// @param userdata Указатель на произвольные данные
/// @param event Событие 
/// @return <Надо читать>
int event_handler(void *userdata, SDL_Event *event);

/// @brief Создаёт очередь для перерисовки
void queue_redraw();

#define exit_on_sdl_error() { printf("%s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); cpptrace::from_current_exception().print(); throw std::runtime_error(""); }
#define warn_on_sdl_error() { printf("внимание: %s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); }

#endif // MAIN_H
