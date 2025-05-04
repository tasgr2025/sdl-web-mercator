#include "main.h"

/// @brief Флаги инициализации SDL
static const int IMG_INIT_EVERYTHING = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;

/// @brief Флаги инициализации SDL
static const Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;

/// @brief Размер поверхности для рисования
vec2 canvas_size { 1280.0f, 768.0f };

/// @brief Текущая позиция WebMercator
dvec3 xyz { 0.0, 0.0, 0.0 };

/// @brief Размер плитки
vec2 tile_size { 256.0f, 256.0f };

/// @brief Шаблон адреса для получения плиток
std::string base_url{"https://tile.openstreetmap.org/{2}/{0}/{1}.png"};

/// @brief Шаг масштабирования
double zoom_step = 0.05;

/// @brief Минимальный уровень детализации
int min_zoom = 0;

/// @brief Максимальный уровень детализации
int max_zoom = 20;

/// @brief Флаг. Перетаскивания мышкой активно
bool dragging { false };

/// @brief Позиция перетаскивания мышкой
dvec2 drag_pos { 0.0, 0.0 };

/// @brief <...>
bool rollover { false };

/// @brief Загруженные плитки
std::unordered_map<int64_t, SDLTile*> cache;

/// @brief Очередь запросов плиток
std::unordered_map<int64_t, ivec4> queue;

/// @brief Данные изображений плиток
std::unordered_map<int64_t, std::string> data;

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


int64_t get_next_in_queue() {
    int64_t idx = -1;
    int tick = 0;
    for (const auto& item : queue) {
        if (item.second.w > tick) {
            idx = item.first;
            tick = item.second.w;
        }
    }
    return idx;
}


int64_t get_next_in_cache() {
    int tick = -1;
    int64_t idx = 0;
    for (const auto& item : cache) {
        int item_tick = item.second->get_tick();
        if (item_tick > tick) {
            tick = item_tick;
            idx = item.first;
        }
    }
    return idx;
}


void url_thread_proc(void* arg) {
    while (run_thread) {
        size_t sz = 0U;
        mutex.lock();
        sz = queue.size();
        mutex.unlock();
        if (sz == 0) {
            std::unique_lock<std::mutex> lk(cv_mutex);
            cv.wait(lk, []{ return resume_thread; });
            resume_thread = false;
            lk.unlock();
            cv.notify_one();
            continue;
        }
        mutex.lock();
        int64_t idx = get_next_in_queue();
        if (idx < 0) {
            mutex.unlock();
            continue;
        }
        ivec4 t = queue[idx];
        queue.erase(idx);
        mutex.unlock();
        auto args = std::make_format_args(t.x, t.y, t.z);
        std::string url = std::vformat(base_url, args);
        std::string img;
        if (!get_url_data(url, img)) {
            continue;
        }
        mutex.lock();
        data[idx] = img;
        mutex.unlock();
        queue_redraw();
    }
}


void draw_map(SDL_Renderer* render) {
    double z = min(double(max_zoom), xyz.z);
    dvec2 t1 = screen_to_tile(z, xyz, canvas_size, {0.0, 0.0});
    dvec2 t2 = screen_to_tile(z, xyz, canvas_size, canvas_size) + dvec2{1.0, 1.0};
    for (auto x = t1.x; x < t2.x; x += 1) {
        for (auto y = t1.y; y < t2.y; y += 1) {
            draw_tile(render, x, y, z);
        }
    }
    return;
}


bool draw_tile(SDL_Renderer* render, double tx, double ty, double z) {
    double tz = floor(z);
    dvec2 p1 = tile_to_screen(xyz, canvas_size, vec3{tx,        ty,        z});
    dvec2 p2 = tile_to_screen(xyz, canvas_size, vec3{tx + 1.0f, ty + 1.0f, z});
    SDLTile* tile = get_tile(tx, ty, tz);
    bool rc = false;
    if (tile) {
        if (tile->get_texture()) {
            dvec2 p3 = p2 - p1;
            SDL_Rect rect_dst(round(p1.x), round(p1.y), round(p3.x), round(p3.y));
            SDL_RenderCopy(render, tile->get_texture(), NULL, &rect_dst);
            rc = true;
        }
        else {
            double zzz = tz;
            double txx = tx;
            double tyy = ty;
            while (zzz >= 1.0) {
                zzz -= 1.0;
                txx = floor(txx / 2.0);
                tyy = floor(tyy / 2.0);
                if (draw_subtile(render, round(txx), round(tyy), round(zzz), round(tx), round(ty), tz)) {
                    rc = true;
                    break;
                }
            }
        }
    }
    return rc;
}


bool draw_subtile(SDL_Renderer* render, int tx, int ty, int tz, int origx, int origy, float origz) {
    SDLTile* tile = get_tile(tx, ty, tz);
    if (!tile) {
        return false;
    }
    SDL_Texture* texture = tile->get_texture();
    if (!texture) {
        return false;
    }

    dvec2 p1 = tile_to_screen(xyz, canvas_size, {origx,       origy,       origz});
    dvec2 p2 = tile_to_screen(xyz, canvas_size, {origx + 1.0, origy + 1.0, origz});
    dvec2 x1 = tile_to_screen(xyz, canvas_size, {tx,          ty,          tz});
    dvec2 x2 = tile_to_screen(xyz, canvas_size, {tx + 1.0,    ty + 1.0,    tz});

    double xdiff = x2.x - x1.x;
    double xrat1 = (p1.x - x1.x) / xdiff;
    double xrat2 = (p2.x - x1.x) / xdiff;
    double xwidth = xrat2 - xrat1;

    double ydiff = x2.y - x1.y;
    double yrat1 = (p1.y - x1.y) / ydiff;
    double yrat2 = (p2.y - x1.y) / ydiff;
    double yheight = yrat2 - yrat1;

    SDL_Point tile_size = tile->get_size();
    SDL_Rect rect_src(
        round(xrat1 * tile_size.x),
        round(yrat1 * tile_size.y),
        round(xwidth * tile_size.x),
        round(yheight * tile_size.y));
    dvec2 dp = p2 - p1;
    SDL_Rect rect_dst(round(p1.x), round(p1.y), round(dp.x), round(dp.y));
    SDL_RenderCopy(render, texture, &rect_src, &rect_dst);
    return true;
}


SDLTile* get_tile(int x, int y, int z) {
    int n = pow(2.0, z);
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

    int64_t idx = tile_to_index(x, y, z);
    Uint32 tick = SDL_GetTicks();

    auto cache_item = cache.find(idx);
    if (cache_item != cache.end()) {
        cache_item->second->set_tick(tick);
        return cache_item->second;
    }

    mutex.lock();
    auto queue_item = queue.find(idx);
    if (queue_item == queue.end()) {
        queue[idx] = ivec4(x, y, z, tick);
    }
    SDLTile* tile = new SDLTile(x, y, z, tick);
    cache[idx] = tile;
    mutex.unlock();
    url_thread_resume();
    return tile;
}


int event_handler(void *userdata, SDL_Event *event) {
    ivec2 mp;
    Uint32 ms = SDL_GetMouseState(&mp.x, &mp.y);
    rollover = (mp.x > 0) && (mp.x < canvas_size.x) && (mp.y > 0) && (mp.y < canvas_size.y);
    dragging &= rollover;
    
    if (!rollover) {
        return 0;
    }

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN:
        dragging = (SDL_BUTTON(1) & ms) > 0;
        drag_pos = screen_to_world(xyz, canvas_size, mp);
        break;
    case SDL_MOUSEBUTTONUP:
        dragging = false;
        break;
    case SDL_MOUSEMOTION:
        if (dragging) {
            dvec2 diff = drag_pos - screen_to_world(xyz, canvas_size, mp);
            xyz.x += diff.x;
            xyz.y += diff.y;
            queue_redraw();
        }
        break;
    case SDL_MOUSEWHEEL:
        step_zoom(xyz, canvas_size, zoom_step * float(event->wheel.y), mp);
        queue_redraw();
        break;
    }
    return 0;
}


void queue_redraw() {
    SDL_Event event;
    event.window.event = SDL_WINDOWEVENT_EXPOSED;
    event.type = SDL_WINDOWEVENT;
    SDL_PushEvent(&event);
    return;
}


bool get_url_data(const std::string& url, std::string& img_data) {
    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.error.code != cpr::ErrorCode::OK) {
        return false;
    }
    img_data = r.text;
    return true;
}


bool update_cache(SDL_Renderer *render)
{
    bool rc = false;
    if (!mutex.try_lock()) {
        return false;
    }
    if (!data.size()) {
        mutex.unlock();
        return false;
    }

    std::vector<int64_t> idxs_erase;
    for (const auto& it: data) {
        if (cache.find(it.first) == cache.end()) {
            continue;
        }
        cache[it.first]->set_texture_from_data(render, &it.second[0], it.second.size());
        idxs_erase.push_back(it.first);
    }
    for (const auto& idx: idxs_erase) {
        data.erase(idx);
    }

    mutex.unlock();
    return true;
}


void main_loop(SDL_Renderer *render)
{
    SDL_Event sdle;
    sdle.type = SDL_FIRSTEVENT;
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
                if (update_cache(render)) {
                    queue_redraw();
                }
            }
        }
    }
}


void url_thread_stop() {
    url_thread_resume();
    run_thread = false;
    url_thread->join();
}


void url_thread_resume() {
    {   std::lock_guard<std::mutex> lk(cv_mutex);
        resume_thread = true; }
    cv.notify_one();
}


int main(int argc, char* argv[]) { CPPTRACE_TRY
{
    CLI::App app{ "Отображает карту в проекции WebMercator" };
    argv = app.ensure_utf8(argv);
    app.add_option("--canvas-size",  canvas_size,             "Размер поверхности для отображения карты в пикселях по горизонтали и вертикали.")->expected(0, 2)->capture_default_str();
    app.add_option("--tile-size",    tile_size,               "Размер плитки (фрагмента) карты в пикселях по горизонтали и вертикали.")->expected(0, 2)->capture_default_str();
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

    std::thread new_thread {url_thread_proc, nullptr};
    url_thread = &new_thread;
    SDL_AddEventWatch(event_handler, nullptr);
    main_loop(render);
    SDL_DestroyWindow(sdlw);
    SDL_Quit();
    url_thread_stop();
    exit(EXIT_SUCCESS);
}
CPPTRACE_CATCH(const std::exception& e) {
    cpptrace::from_current_exception().print();
    }
}
