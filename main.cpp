#include "main.h"


int event_handler(void *userdata, SDL_Event *event)
{
    if (event->type == SDL_MOUSEMOTION)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        printf("x:%d y:%d\n", x, y);
    }
    return 0;
}


int main()
{
    auto ofstream = std::ofstream("tmp1.png", std::ios::out | std::ios::binary);
    auto session = cpr::Session();
    session.SetUrl(cpr::Url{"https://a.tile.openstreetmap.org/0/0/0.png"});
    auto response = session.Download(ofstream);
    printf("response.downloaded_bytes: %lld\n", response.downloaded_bytes);
    ofstream.flush();
    ofstream.close();
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
        800, 600, SDL_WINDOW_OPENGL);
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
    SDL_Renderer *render = SDL_CreateRenderer(sdlw, -1, render_flags);
    SDL_AddEventWatch(event_handler, nullptr);
    SDL_Event sdle;
    while (1)
    {
        int rc = SDL_WaitEvent(&sdle);
        if (rc == 0)
        {
            const char* err_str = SDL_GetError();
            printf("%s:%u: sdl2:\"%s\"\n", __FILE__, __LINE__,  err_str);
            break;
        }
        if (sdle.type == SDL_QUIT)
        {
            break;
        }
        else if (sdle.type == SDL_WINDOWEVENT)
        {
            if (sdle.window.event == SDL_WINDOWEVENT_EXPOSED)
            {
                SDL_RenderClear(render);
                SDL_RenderPresent(render);
            }
        }
    }
    SDL_DestroyWindow(sdlw);
    SDL_Quit();
    exit(EXIT_SUCCESS);
}
