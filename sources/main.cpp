#include "main.h"

/// @brief Флаги инициализации SDL
static const int IMG_INIT_EVERYTHING = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;

/// @brief Флаги инициализации SDL
static const Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;

/// @brief Размер поверхности для рисования
vec2 canvas_size { 1024.0f, 768.0f };

/// @brief Текущая позиция WebMercator
vec3 xyz { 0.0f, 0.0f, 0.0f };

/// @brief Размер плитки
vec2 tile_size { get_tile_size()};

/// @brief Шаблон адреса для получения плиток
std::string base_url("https://a.tile.openstreetmap.org/{0}/{1}/{2}.png");

/// @brief Шаг масштабирования
float zoom_step = 0.025;

/// @brief Флаг. Перетаскивания мышкой активно
bool dragging { false };

/// @brief Позиция перетаскивания мышкой
vec2 drag_pos { 0.0f, 0.0f };

/// @brief <...>
bool rollover { false };


int event_handler(void *userdata, SDL_Event *event) {
    ivec2 mouse_pos;
    Uint32 ms = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    rollover = (mouse_pos.x > 0) && (mouse_pos.x < canvas_size.x) && (mouse_pos.y > 0) && (mouse_pos.y < canvas_size.y);
    dragging &= rollover;
    
    if (!rollover) {
        return 0;
    }

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        dragging = (SDL_BUTTON(1) & ms) > 0;
        drag_pos = screen_to_world(xyz, canvas_size, mouse_pos);
    }

    if (event->type == SDL_MOUSEMOTION) {
        if (dragging) {
            vec2 diff = drag_pos - screen_to_world(xyz, canvas_size, mouse_pos);
            xyz.x += diff.x;
            xyz.y += diff.y;
            queue_redraw();
        }
    }
    
    printf("dragging:%s\n", dragging ? "да" : "нет");
    return 0;
}


void queue_redraw() {
    return;
}


int main(int argc, char* argv[]) { CPPTRACE_TRY
{
    CLI::App app{ "Отображает карту в проекции WebMercator" };
    argv = app.ensure_utf8(argv);
    app.add_option("--canvas-size", canvas_size, "Размер поверхности для отображения карты в пикселях по горизонтали и вертикали.")->expected(0, 2);
    app.add_option("--tile-size",   tile_size,   "Размер плитки (фрагмента) карты в пикселях по горизонтали и вертикали.")->expected(0, 2);
    app.add_option("--base-url",    base_url,    "Шаблон адреса для получения плиток где вместо {0}, {1}, {2} будут подставлены x, y, z соответственно.")->capture_default_str();
    app.add_option("--zoom_step",   zoom_step,   "Шаг масштабирования.")->capture_default_str();
    CLI11_PARSE(app, argc, argv);

    SDL_version ver;
    SDL_GetVersion(&ver);
    printf("Версия SDL:\"%u.%u.%u\"\n", ver.major, ver.minor, ver.patch);
    int rc = SDL_Init(SDL_INIT_EVERYTHING);
    if (rc < 0) {
        exit_on_sdl_error();
    }
    SDL_Window* sdlw = SDL_CreateWindow(
        "Обзор карты WEB Mercator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        canvas_size.x, canvas_size.y, SDL_WINDOW_OPENGL);
    SDL_Renderer *render = SDL_CreateRenderer(sdlw, -1, render_flags);
    if (!render) {
        exit_on_sdl_error();
    }
    rc = IMG_Init(IMG_INIT_EVERYTHING);
    if (rc != IMG_INIT_EVERYTHING) {
        warn_on_sdl_error();
    }

    SDLTile st;
    cpr::Response r = cpr::Get(cpr::Url{st.get_url(base_url)});
    if (r.error.code != cpr::ErrorCode::OK) {
        printf("%s:\"%s\"\n", r.error.message.c_str(), r.url.c_str());
        exit_on_sdl_error();
    }
    SDL_RWops* rwop = SDL_RWFromConstMem(r.text.data(), r.text.size());
    if (!rwop) {
        exit_on_sdl_error();
    }
    SDL_Surface *surface = IMG_Load_RW(rwop, 0);
    if (!surface) {
        exit_on_sdl_error();
    }
    rc = SDL_RWclose(rwop);
    if (rc) {
        exit_on_sdl_error();
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(render, surface);
    if (!texture) {
        exit_on_sdl_error();
    }
    SDL_FreeSurface(surface);

    vec2 tile_size = get_tile_size();
    SDL_Rect dst_rect {
        static_cast<int>((canvas_size.x - tile_size.x) / 2.0f),
        static_cast<int>((canvas_size.y - tile_size.y) / 2.0f),
        static_cast<int>(tile_size.x),
        static_cast<int>(tile_size.y)
    };
    SDL_AddEventWatch(event_handler, nullptr);
    SDL_Event sdle = {0};
    while (sdle.type != SDL_QUIT) {
        int rc = SDL_WaitEvent(&sdle);
        if (rc == 0) {
            exit_on_sdl_error();
        }
        else if (sdle.type == SDL_WINDOWEVENT) {
            if (sdle.window.event == SDL_WINDOWEVENT_EXPOSED) {
                SDL_RenderClear(render);
                SDL_RenderCopy(render, texture, NULL, &dst_rect);
                SDL_RenderPresent(render);
            }
        }
    }

    SDL_DestroyWindow(sdlw);
    SDL_Quit();
    exit(EXIT_SUCCESS);
} CPPTRACE_CATCH(const std::exception& e) {
    cpptrace::from_current_exception().print();
    }
}
