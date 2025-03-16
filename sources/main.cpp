#include "main.h"

/// @brief Флаги инициализации SDL
static const int IMG_INIT_EVERYTHING = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;

/// @brief Флаги инициализации SDL
static const Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;

/// @brief Размер поверхности для рисования
vec2 canvas_size { 512.0f, 512.0f };

vec2 window_size { 512.0f, 512.0f };

/// @brief Текущая позиция WebMercator
vec3 xyz { 0.0f, 0.0f, 1.0f };

/// @brief Размер плитки
vec2 tile_size { get_tile_size()};

/// @brief Шаблон адреса для получения плиток
std::string base_url{"https://a.tile.openstreetmap.org/{0}/{1}/{2}.png"};

/// @brief Шаг масштабирования
float zoom_step = 0.025;

/// @brief Минимальный уровень детализации
int min_zoom = 1.0f;

/// @brief Максимальный уровень детализации
int max_zoom = 18.0f;

/// @brief Флаг. Перетаскивания мышкой активно
bool dragging { false };

/// @brief Позиция перетаскивания мышкой
vec2 drag_pos { 0.0f, 0.0f };

/// @brief <...>
bool rollover { false };

/// @brief 
std::unordered_map<int, SDLTile*> requests;

/// @brief
std::unordered_map<int, SDLTile*> cache;

/// @brief
std::unordered_map<int, SDLTile*> queue;

/// @brief 
Uint32 last_cache_check = 0;

/// @brief
size_t max_concurrent_requests = 1;

/// @brief 
size_t req_count = 0;


void clean_cache() {
    return;
}


void draw_map(SDL_Renderer* render) {
    vec2 t1 = screen_to_tile(xyz, canvas_size, vec2{0.0f, 0.0f});
    vec2 t2 = screen_to_tile(xyz, canvas_size, canvas_size) + vec2{1.0f, 1.0f};
    printf(">>> %0.1f..%0.1f (%0.3f) xyz.x:%0.3f xyz.z:%0.3f\n", t1.x, t2.x, t1.x - t2.x, xyz.x, xyz.z);
    for (float x = t1.x; x < t2.x; x += 1.0f)
    for (float y = t1.y; y < t2.y; y += 1.0f) {
            draw_tile(render, fmodf(x, t2.x - t1.x), y, 0.0f);
        }
    printf("<<<<\n");
    return;
}


SDLTile* get_next_in_queue() {
    SDLTile* tile = nullptr;
    for (const auto &[idx, t] : queue) {
        if ((!tile) || (t->get_tick() > tile->get_tick())) {
            tile = t;
        }
    }
    return tile;
}


void draw_tile(SDL_Renderer* render, float tx, float ty, float tz) {
    tz = floorf(tz);
    vec2 p1 = tile_to_screen(xyz, canvas_size, vec3{tx,        ty,        tz});
    vec2 p2 = tile_to_screen(xyz, canvas_size, vec3{tx + 1.0f, ty + 1.0f, tz});
    SDLTile* tile = get_tile(tx, ty, tz);
    printf("tile:%p\n", tile);
    if (tile) {
        if (tile->get_texture()) {
            vec2 p3 = p2 - p1;
            SDL_Rect rect_dst(p1.x, p1.y, p3.x, p3.y);
            SDL_RenderCopy(render, tile->get_texture(), NULL, &rect_dst);
        }
        else {
            float zzz = tz;
            float txx = tx;
            float tyy = ty;
            while (zzz > 1.0f) {
                zzz -= 1.0f;
                txx = floorf(txx / 2.0f);
                tyy = floorf(tyy / 2.0f);
                if (draw_subtile(render, txx, tyy, zzz, tx, ty, tz)) {
                    break;
                }
            }
        }
    }
}


bool draw_subtile(SDL_Renderer* render, int tx, int ty, int tz, int origx, int origy, float origz) {
    SDLTile* subtile = get_tile(tx, ty, tz);
    if (!subtile) {
        return false;
    }

    vec2 p1 = tile_to_screen(xyz, canvas_size, vec3{float(origx),     float(origy),     float(origz)});
    vec2 p2 = tile_to_screen(xyz, canvas_size, vec3{float(origx + 1), float(origy + 1), float(origz)});
    vec2 x1 = tile_to_screen(xyz, canvas_size, vec3{float(tx),        float(ty),        float(tz)});
    vec2 x2 = tile_to_screen(xyz, canvas_size, vec3{float(tx + 1),    float(ty + 1),    float(tz)});

    float xdiff = x2.x - x1.x;
    float xrat1 = (p1.x - x1.x) / xdiff;
    float xrat2 = (p2.x - x1.x) / xdiff;
    float xwidth = xrat2 - xrat1;

    float ydiff = x2.y - x1.y;
    float yrat1 = (p1.y - x1.y) / ydiff;
    float yrat2 = (p2.y - x1.y) / ydiff;
    float yheight = yrat2 - yrat1;

    vec2 tile_size = get_tile_size();
    SDL_Rect rect_dst ( xrat1 * tile_size.x, yrat1 * tile_size.y, xwidth * tile_size.x, yheight * tile_size.y );
    SDL_Texture* texture = subtile->get_texture();
    if (texture) {
        vec2 dp = p2 - p1;
        SDL_Rect rect_src (p1.x, p1.y, dp.x, dp.y);
        SDL_RenderCopy(render, texture, &rect_src, &rect_dst);
        return true;
    }
    return false;
}


SDLTile* get_tile(int x, int y, int z) {
    float n = powf(2.0f, z);
    if (x >= 0) {
        x %= int(n);
    }
    else {
        x = -x - 1;
        x = int(n - 1) - (x % int(n));
    }

    // за пределами
    if ((z < 0) || (y < 0) || (y >= n)) {
        return nullptr;
    }

    int idx = tile_to_index(x, y, z);
    Uint32 tick = SDL_GetTicks();

    // retrieve from current requests
    auto item = requests.find(idx);
    if (item != requests.end()) {
        item->second->set_tick(tick);
        return item->second;
    }

    item = cache.find(idx);
    if (item != cache.end()) {
        item->second->set_tick(tick);
        return item->second;
    }

    item = queue.find(idx);
    if (item != queue.end()) {
        item->second->set_tick(tick);
        return item->second;
    }

    SDLTile* tile = new SDLTile(idx, x, y, z);
    tile->set_tick(tick);
    queue[idx] = tile;
    return tile;
}


int event_handler(void *userdata, SDL_Event *event) {
    ivec2 mouse_pos;
    Uint32 ms = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    rollover = (mouse_pos.x > 0) && (mouse_pos.x < canvas_size.x) && (mouse_pos.y > 0) && (mouse_pos.y < canvas_size.y);
    dragging &= rollover;
    
    if (!rollover) {
        return 0;
    }

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN:
        dragging = (SDL_BUTTON(1) & ms) > 0;
        drag_pos = screen_to_world(xyz, canvas_size, mouse_pos);
        break;
    case SDL_MOUSEBUTTONUP:
        dragging = false;
        break;
    case SDL_MOUSEMOTION:
        if (dragging) {
            vec2 diff = drag_pos - screen_to_world(xyz, canvas_size, mouse_pos);
            xyz.x += diff.x;
            xyz.y += diff.y;
            xyz.x = fmodf(xyz.x, canvas_size.x / tile_size.x);
            queue_redraw(event);
        }
        break;
    case SDL_MOUSEWHEEL:
        multiply_zoom(xyz, canvas_size, 1.0f + zoom_step * float(event->wheel.y), mouse_pos);
        queue_redraw(event);
        break;
    }
    return 0;
}


void queue_redraw(SDL_Event* event) {
    event->window.event = SDL_WINDOWEVENT_EXPOSED;
    event->type = SDL_WINDOWEVENT;
    SDL_PushEvent(event);
    return;
}


int main(int argc, char* argv[]) { CPPTRACE_TRY
{
    CLI::App app{ "Отображает карту в проекции WebMercator" };
    argv = app.ensure_utf8(argv);
    app.add_option("--canvas-size",  canvas_size,             "Размер поверхности для отображения карты в пикселях по горизонтали и вертикали.")->expected(0, 2)->capture_default_str();
    app.add_option("--tile-size",    tile_size,               "Размер плитки (фрагмента) карты в пикселях по горизонтали и вертикали.")->expected(0, 2)->capture_default_str();
    app.add_option("--window-size",  window_size,             "Размер окна программы в пикселях по горизонтали и вертикали.")->expected(0, 2)->capture_default_str();
    app.add_option("--base-url",     base_url,                "Шаблон адреса для получения плиток где вместо {0}, {1}, {2} будут подставлены x, y, z соответственно.")->capture_default_str();
    app.add_option("--zoom-step",    zoom_step,               "Шаг масштабирования.")->capture_default_str();
    app.add_option("--max-zoom",     max_zoom,                "Максимальный масштаб.")->capture_default_str();
    app.add_option("--max-requests", max_concurrent_requests, "Максимальное количесво запорсов одновременно")->capture_default_str();
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
        window_size.x, window_size.y, SDL_WINDOW_OPENGL);
    SDL_Renderer *render = SDL_CreateRenderer(sdlw, -1, render_flags);
    if (!render) {
        exit_on_sdl_error();
    }
    rc = IMG_Init(IMG_INIT_EVERYTHING);
    if (rc != IMG_INIT_EVERYTHING) {
        warn_on_sdl_error();
    }

    SDLTile tile;
    cpr::Response r = cpr::Get(cpr::Url{tile.get_url(base_url)});
    if (r.error.code != cpr::ErrorCode::OK) {
        printf("%s:\"%s\"\n", r.error.message.c_str(), r.url.c_str());
        exit_on_sdl_error();
    }
    tile.set_texture_from_data(render, r.text.data(), r.text.size());
    int idx = tile_to_index(0, 0, 0);
    cache[idx] = &tile;
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
                draw_map(render);
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
