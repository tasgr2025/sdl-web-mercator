#include "sdltile.h"


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
    int w{0}, h{0};
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    return SDL_Point{w, h};
}
