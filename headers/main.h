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
void queue_redraw(SDL_Event* event);

/// @brief 
/// @param tx 
/// @param ty 
/// @param tz 
/// @param origx 
/// @param origy 
/// @param origz 
/// @return 
bool draw_subtile(SDL_Renderer* render, int tx, int ty, int tz, int origx, int origy, float origz);

/// @brief Получить плитку из всех возможных источников
/// @param x 
/// @param y 
/// @param z 
/// @return 
SDLTile* get_tile(int x, int y, int z);

/// @brief Получить плитку из очереди с самой новой меткой времени
/// @return 
SDLTile* get_next_in_queue();

/// @brief 
/// @param render 
/// @param tx 
/// @param ty 
/// @param tz 
void draw_tile(SDL_Renderer* render, float tx, float ty, float tz);

/// @brief
/// @param render
void draw_map(SDL_Renderer* render);

/// @brief
void clean_cache();

#endif // MAIN_H
