#include "main.h"

/// @brief Флаги инициализации SDL
static const int IMG_INIT_EVERYTHING = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;

/// @brief Шаблон адреса для загрузки плиток
static const char* base_url = "https://tile.openstreetmap.org/{}/{}/{}.png";

/// @brief Размер поверхности для рисования
vec2 canvas_size {1024.0f, 768.0f};

/// @brief Текущая позиция WebMercator
vec3 xyz {0.0f, 0.0f, 0.0f};


bool SDLTile::set_texture_from_data(SDL_Renderer *render, char *data, const size_t len) {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    SDL_RWops* rwop = SDL_RWFromMem(data, len);
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    if (!surface) {
        return false;
    }
    texture = SDL_CreateTextureFromSurface(render, surface);
    SDL_FreeSurface(surface);
    return true;
}


int event_handler(void *userdata, SDL_Event *event)
{
    static uint32_t tick0;
    if (event->type == SDL_MOUSEMOTION)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        uint32_t tick1 = SDL_GetTicks();
        uint32_t dtick = tick1 - tick0;
        tick0 = tick1;
        vec2 screen_coords {x, y};
        vec2 ll = screen_to_lonlat (xyz, canvas_size, screen_coords);
        printf("%u x:%d y:%d lon:%f lat:%f\n", dtick, x, y, ll.x, ll.y);
    }
    return 0;
}


int main(int argc, char* argv[])
{
    SDL_version ver;
    SDL_GetVersion(&ver);
    printf("sdl version:\"%u.%u.%u\"\n", ver.major, ver.minor, ver.patch);
    int rc = SDL_Init(SDL_INIT_EVERYTHING);
    if (rc < 0)
    {
        const char* err_str = SDL_GetError();
        printf("%s:%u: sdl2:\"%s\"\n", __FILE__, __LINE__,  err_str);
        exit(rc);
    }
    
    SDL_Window* sdlw = SDL_CreateWindow(
        "Обзор карты WEB Mercator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        canvas_size.x, canvas_size.y, SDL_WINDOW_OPENGL);
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
    SDL_Renderer *render = SDL_CreateRenderer(sdlw, -1, render_flags);

    rc = IMG_Init(IMG_INIT_EVERYTHING);
    if (rc != IMG_INIT_PNG)
    {
        const char* err_str = SDL_GetError();
        printf("%s:%u: sdl2:\"%s\"\n", __FILE__, __LINE__,  err_str);
        exit(1);
    }
    cpr::Response r = cpr::Get(cpr::Url{"https://a.tile.openstreetmap.org/0/0/0.png"});
    SDL_RWops* rwop = SDL_RWFromMem(r.text.data(), r.text.size());
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    SDL_Texture *texture = NULL;
    if (surface)
    {
        texture = SDL_CreateTextureFromSurface(render, surface);
        SDL_FreeSurface(surface);
        exit(1);
    }

    vec2 tile_size = get_tile_size();
    SDL_Rect dst_rect
    {
        static_cast<int>((canvas_size.x - tile_size.x) / 2.0f),
        static_cast<int>((canvas_size.y - tile_size.y) / 2.0f),
        static_cast<int>(tile_size.x),
        static_cast<int>(tile_size.y)
    };

    SDL_AddEventWatch(event_handler, nullptr);
    SDL_Event sdle = {0};
    while (sdle.type != SDL_QUIT)
    {
        int rc = SDL_WaitEvent(&sdle);
        if (rc == 0)
        {
            const char* err_str = SDL_GetError();
            printf("%s:%u: sdl2:\"%s\"\n", __FILE__, __LINE__,  err_str);
            break;
        }
        else if (sdle.type == SDL_WINDOWEVENT)
        {
            if (sdle.window.event == SDL_WINDOWEVENT_EXPOSED)
            {
                SDL_RenderClear(render);
                SDL_RenderCopy(render, texture, NULL, &dst_rect);
                SDL_RenderPresent(render);
            }
        }
    }

    SDL_DestroyWindow(sdlw);
    SDL_Quit();
    exit(EXIT_SUCCESS);
}
