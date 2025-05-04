#include "sdltile.h"


bool SDLTile::set_texture_from_data(SDL_Renderer *render, const char *data, const size_t len) {
    SDL_RWops* rwop = SDL_RWFromConstMem(data, len);
    if (!rwop) {
        return false;
    }
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    if (!surface) {
        return false;
    }
    SDL_RWclose(rwop);

    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    texture = SDL_CreateTextureFromSurface(render, surface);
    if (!texture) {
        return false;
    }
    SDL_FreeSurface(surface);
    return true;
}


SDL_Point SDLTile::get_size() {
    SDL_Point p;
    SDL_QueryTexture(texture, NULL, NULL, &p.x, &p.y);
    return p;
}
