#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cpr/cpr.h>

#include "tiletools.h"

/// @brief Обработчик событий SDL
/// @param userdata Указатель на произвольные данные
/// @param event Событие 
/// @return <Надо читать>
int event_handler(void *userdata, SDL_Event *event);


/// @brief Шаблон адреса для загрузки плиток
static const char* base_url = "https://tile.openstreetmap.org/{}/{}/{}.png";

class SDLTile {
    uint32_t i, x, y, z, t;
    SDL_Texture* texture;
public:
    std::string get_url() { return std::vformat(base_url, std::make_format_args(x, y, z)); }
    SDLTile (uint32_t i0, uint32_t x0, uint32_t y0, uint32_t z0): i{i0}, x{x0}, y{y0}, z{z0}, texture{nullptr} {}
    bool set_texture_from_data(SDL_Renderer *render, const char *data, const size_t len);
    SDL_Texture* get_texture() { return texture; }
    ~SDLTile () { SDL_DestroyTexture(texture); }
};

#endif // MAIN_H
