#include "main.h"

/// @brief Флаги инициализации SDL
static const int IMG_INIT_EVERYTHING = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;

/// @brief Флаги инициализации SDL
static const Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;

/// @brief Размер поверхности для рисования
vec2 canvas_size { 512.0f, 512.0f };

vec2 window_size { 512.0f, 512.0f };

/// @brief Текущая позиция WebMercator
vec3 xyz { 0.0f, 0.0f, 0.0f };

/// @brief Размер плитки
vec2 tile_size { get_tile_size()};

/// @brief Шаблон адреса для получения плиток
std::string base_url{"https://tile.openstreetmap.org/{2}/{0}/{1}.png"};

/// @brief Шаг масштабирования
float zoom_step = 0.05;

/// @brief Минимальный уровень детализации
int min_zoom = 0.0f;

/// @brief Максимальный уровень детализации
int max_zoom = 3.0f;

/// @brief Флаг. Перетаскивания мышкой активно
bool dragging { false };

/// @brief Позиция перетаскивания мышкой
vec2 drag_pos { 0.0f, 0.0f };

/// @brief <...>
bool rollover { false };

/// @brief
std::unordered_map<int, SDLTile*> cache;

/// @brief
std::vector<SDLTile> queue;

std::mutex mutex;
std::mutex cv_mutex;
std::condition_variable cv;
bool resume_thread { false };
std::atomic<bool> run_thread { true };
std::thread* url_thread = nullptr;

void clean_cache() {
    for (auto pair: cache) {
        delete pair.second;
    }
    cache.clear();
}


void url_thread_proc(void* arg) {
    while (run_thread) {
        mutex.lock();
        size_t sz = queue.size();
        mutex.unlock();
        if (sz == 0) {
            std::unique_lock<std::mutex> lk(cv_mutex);
            cv.wait(lk, []{ return resume_thread; });
            resume_thread = false;
            lk.unlock();
            cv.notify_one();
            continue;
        }
    }
}


void draw_map(SDL_Renderer* render) {
    float z = min(float(max_zoom), xyz.z);
    vec2 t1 = screen_to_tile(z, xyz, canvas_size, vec2{0.0f, 0.0f});
    vec2 t2 = screen_to_tile(z, xyz, canvas_size, canvas_size) + vec2{1.0f, 1.0f};
    printf(">>> %0.1f..%0.1f (%0.3f) xyz.x:%0.3f xyz.z:%0.3f\n", t1.x, t2.x, t1.x - t2.x, xyz.x, xyz.z);
    for (int x = int(t1.x); x < int(t2.x); x += 1) {
        for (int y = int(t1.y); y < int(t2.y); y += 1) {
            draw_tile(render, x, y, z);
        }
    }
    printf("<<<\n");
    return;
}


void draw_tile(SDL_Renderer* render, int tx, int ty, float z) {
    float tz = floorf(z);
    vec2 p1 = tile_to_screen(xyz, canvas_size, vec3{tx,        ty,        z});
    vec2 p2 = tile_to_screen(xyz, canvas_size, vec3{tx + 1.0f, ty + 1.0f, z});
    SDLTile* tile = get_tile(tx, ty, tz);
    if (tile) {
        if (tile->get_texture()) {
            vec2 p3 = p2 - p1;
            SDL_Rect rect_dst(p1.x, p1.y, p3.x, p3.y);
            printf("tile: %d %d\n", rect_dst.x, rect_dst.y);
            SDL_RenderCopy(render, tile->get_texture(), NULL, &rect_dst);
        }
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
    SDL_Rect rect_src ( xrat1 * tile_size.x, yrat1 * tile_size.y, xwidth * tile_size.x, yheight * tile_size.y );
    SDL_Texture* texture = subtile->get_texture();
    if (texture) {
        vec2 dp = p2 - p1;
        SDL_Rect rect_dst (p1.x, p1.y, dp.x, dp.y);
        printf("sub_tile: %d %d\n", rect_dst.x, rect_dst.y);
        SDL_RenderCopy(render, texture, &rect_src, &rect_dst);
        return true;
    }
    return false;
}


SDLTile* get_tile(int x, int y, int z) {
    int n = powf(2.0f, z);
    if (x >= 0) {
        x %= n;
    }
    else {
        x = -x - 1;
        x = (n - 1) - (x % n);
    }

    // за пределами
    if ((z < 0) || (y < 0) || (y >= n)) {
        return nullptr;
    }

    int idx = tile_to_index(x, y, z);
    Uint32 tick = SDL_GetTicks();

    auto item = cache.find(idx);
    if (item != cache.end()) {
        item->second->set_tick(tick);
        return item->second;
    }

    queue.push_back(SDLTile(x, y, z, tick));
    return &queue.back();
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
            queue_redraw(event);
        }
        break;
    case SDL_MOUSEWHEEL:
        step_zoom(xyz, canvas_size, zoom_step * float(event->wheel.y), mouse_pos);
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


bool load_tile(SDL_Renderer* render, SDLTile& tile) {
    std::string url = tile.get_url(base_url);
    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.error.code != cpr::ErrorCode::OK) {
        printf("%s:\"%s\"\n", r.error.message.c_str(), r.url.c_str());
        return false;
    }
    if (!tile.set_texture_from_data(render, r.text.data(), r.text.size())) {
        printf("не загружено: %s\n", tile.get_url(base_url).c_str());
        return false;
    }
    int idx = tile.get_index();
    cache[idx] = &tile;
    printf("загружена плитка: %s\n", tile.get_url(base_url).c_str());
    return true;
}


void main_loop(SDL_Renderer *render) {
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
}


void url_thread_stop() {
    {   std::lock_guard<std::mutex> lk(cv_mutex);
        resume_thread = true; }
    cv.notify_one();
    run_thread = false;
    url_thread->join();
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
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    SDL_Renderer *render = SDL_CreateRenderer(sdlw, -1, render_flags);
    if (!render) {
        exit_on_sdl_error();
    }
    rc = IMG_Init(IMG_INIT_EVERYTHING);
    if (rc != IMG_INIT_EVERYTHING) {
        warn_on_sdl_error();
    }

    int x=0, y=0;
    SDL_GetRendererOutputSize(render, &x, &y);
    printf("размер вывода визуализатора в пикселях: %dx%d\n", x, y);

    std::thread new_thread {url_thread_proc, nullptr};
    url_thread = &new_thread;

    for (int z = 0; z <= max_zoom; z ++) {
        int c = powf(2.0f, float(z));
        for (int y = 0; y < c; y ++) {
            for (int x = 0; x < c; x ++) {
                 queue.emplace_back(SDLTile(x, y, z));
            }
        }
    }

    for (auto& tile: queue) {
        load_tile(render, tile);
        {   std::lock_guard<std::mutex> lk(cv_mutex);
            resume_thread = true; }
        cv.notify_one();
    }
    printf("загружено %zd плиток\n", cache.size());
    SDL_AddEventWatch(event_handler, nullptr);
    main_loop(render);
    SDL_DestroyWindow(sdlw);
    SDL_Quit();
    url_thread_stop();
    exit(EXIT_SUCCESS);
}   CPPTRACE_CATCH(const std::exception& e) {
        cpptrace::from_current_exception().print();
    }
}
