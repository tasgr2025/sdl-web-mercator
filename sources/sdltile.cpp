#include "sdltile.h"


Uint32 SDLTile::get_index() {
    float i = (powf(4.0f, z) - 1.0f) / 3.0f;
    float n = powf(2.0f, z);
    return Uint32(i + (y * n + x));
}


bool SDLTile::set_texture_from_data(SDL_Renderer *render, const char *data, const size_t len) {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    SDL_RWops* rwop = SDL_RWFromConstMem(data, len);
    if (!rwop) {
        return false;
    }
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    if (!surface) {
        return false;
    }
    texture = SDL_CreateTextureFromSurface(render, surface);
    if (!texture) {
        return false;
    }
    SDL_FreeSurface(surface);
    SDL_RWclose(rwop);
    return true;
}


SDL_Point SDLTile::get_size() {
    SDL_Point p;
    SDL_QueryTexture(texture, NULL, NULL, &p.x, &p.y);
    return p;
}
