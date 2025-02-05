cmake_minimum_required(VERSION 3.21)
project(sdl-web-mercator)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS FALSE)
set(BUILD_STATIC_LIBS TRUE)

include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY "https://github.com/libcpr/cpr.git"
    GIT_TAG "1.11.1"
    GIT_PROGRESS    TRUE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "release-2.30.11"
    GIT_PROGRESS    TRUE)

    FetchContent_Declare(
    SDL2_IMAGE
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL_image
    GIT_TAG         "release-2.8.4"
    GIT_PROGRESS    TRUE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG "1.0.1")

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY "https://github.com/CLIUtils/CLI11.git"
    GIT_TAG "v2.4.2")

FetchContent_MakeAvailable(cpr SDL2 SDL2_IMAGE glm CLI11)

include_directories("${sdl2_SOURCE_DIR}/include")
include_directories("${glm_SOURCE_DIR}")
include_directories("${cli11_SOURCE_DIR}/include/CLI")

file(GLOB SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED=1 _USE_MATH_DEFINES=1)

target_link_libraries(${PROJECT_NAME} SDL2::SDL2main SDL2::SDL2-static SDL2_image cpr)
