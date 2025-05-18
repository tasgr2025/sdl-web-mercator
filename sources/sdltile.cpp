#include "sdltile.h"


bool SDLTile::set_texture(SDL_Renderer *render, const std::string& data) {
    SDL_RWops* rwop = SDL_RWFromConstMem(&data[0], data.size());
    if (!rwop) {
        return false;
    }
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    SDL_RWclose(rwop);
    if (!surface) {
        return false;
    }

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
