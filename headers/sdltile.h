#include <cstdint>
#include <cmath>
#include <string>
#include <format>
#include <SDL.h>
#include <SDL_image.h>


class SDLTile {
    Uint32 x{0U}, y{0U}, z{0U}, t{0U};
    SDL_Texture* texture{nullptr};
    Uint32 tick{0U};
public:
    SDLTile() {}
    ~SDLTile () { if (texture) SDL_DestroyTexture(texture); }
    SDLTile(Uint32 x0, Uint32 y0, Uint32 z0): x{x0}, y{y0}, z{z0} {}
    std::string get_url(const std::string& base_url) { return std::vformat(base_url, std::make_format_args(x, y, z)); }
    bool set_texture_from_data(SDL_Renderer *render, const char *data, const size_t len);
    SDL_Texture* get_texture() { return texture; }
    void set_tick(Uint32 tick) { this->tick = tick; }
    Uint32 get_tick() { return tick; }
    SDL_Point get_size();
    Uint32 get_index();
};
