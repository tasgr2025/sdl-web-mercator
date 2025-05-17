#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <cmath>
#include <thread>
#include <SDL.h>
#include <SDL_image.h>
#include <cpr/cpr.h>
#include <CLI.hpp>
#include <cpptrace/from_current.hpp>

#include "sdltile.h"
#include "tiletools.h"

#define exit_on_sdl_error() { printf("%s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); cpptrace::from_current_exception().print(); throw std::runtime_error(""); }
#define warn_on_sdl_error() { printf("внимание: %s:%u: \"%s\"\n", __FILE__, __LINE__, SDL_GetError()); }


/// @brief Возвращает индекс плитки самой новой по времени из очереди загрузки
/// @return Одномерный индекс плитки
uint64_t get_next_in_queue();

/// @brief Возвращает индекс плитки самой новой по времени из кэша
/// @return Одномерный индекс плитки
uint64_t get_next_in_cache();

/// @brief Обработчик событий SDL
/// @param userdata Указатель на произвольные данные
/// @param event Событие 
/// @return <Надо читать>
int event_handler(void *userdata, SDL_Event *event);

/// @brief Создаёт очередь для перерисовки
void queue_redraw();

/// @brief 
/// @param tx 
/// @param ty 
/// @param tz 
/// @param origx 
/// @param origy 
/// @param origz 
/// @return 
bool draw_subtile(SDL_Renderer* render, int tx, int ty, int tz, int origx, int origy, double origz);

/// @brief Получить плитку из всех возможных источников
/// @param x 
/// @param y 
/// @param z 
/// @return 
SDLTile* get_tile(uint64_t x, uint64_t y, uint64_t z);

/// @brief 
/// @param render 
/// @param tx 
/// @param ty 
/// @param tz 
bool draw_tile(SDL_Renderer* render, double tx, double ty, double tz);

/// @brief
/// @param render
void draw_map(SDL_Renderer* render);

/// @brief
void clean_cache();

/// @brief
/// @param render
void main_loop(SDL_Renderer *render);

/// @brief 
/// @param arg 
void url_thread_proc(void* arg);

/// @brief 
/// @param url_thread 
void url_thread_stop();


/// @brief
void url_thread_resume();


/// @brief
/// @param url
/// @param data
bool get_url_data(const std::string& url, std::string& data);

#endif // MAIN_H
