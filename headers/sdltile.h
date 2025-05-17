#include <cstdint>
#include <cmath>
#include <string>
#include <format>
#include <SDL.h>
#include <SDL_image.h>


class SDLTile {
    Uint64 x, y, z, t;
    SDL_Texture* texture { nullptr };
    Uint64 tick{0U};
public:
    SDLTile() {}
    ~SDLTile () { if (texture) SDL_DestroyTexture(texture); }
    SDLTile(Uint64 x0 = 0U, Uint64 y0 = 0U, Uint64 z0 = 0U, Uint64 t0 = 0U): x{x0}, y{y0}, z{z0}, tick{t0} {}
    std::string get_url(const std::string& base_url) { return std::vformat(base_url, std::make_format_args(x, y, z)); }
    bool set_texture(SDL_Renderer *render, const std::string& data);
    SDL_Texture* get_texture() { return texture; }
    void set_tick(Uint32 tick) { this->tick = tick; }
    Uint32 get_tick() { return tick; }
    SDL_Point get_size();
    Uint64 get_zoom() { return z; }
};
