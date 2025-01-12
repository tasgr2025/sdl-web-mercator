cmake_minimum_required(VERSION 3.21)
project(sdl-web-mercator)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS FALSE)
set(BUILD_STATIC_LIBS TRUE)

include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY "https://github.com/libcpr/cpr.git"
    GIT_TAG "1.11.1")

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "release-2.30.11")

FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG "1.0.1")

FetchContent_MakeAvailable(cpr SDL2 glm)

include_directories("${sdl2_SOURCE_DIR}/include")
include_directories("${glm_SOURCE_DIR}")

file(GLOB SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED=1 _USE_MATH_DEFINES=1)

target_link_libraries(${PROJECT_NAME} SDL2::SDL2main SDL2::SDL2-static cpr)
