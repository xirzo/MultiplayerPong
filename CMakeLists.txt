cmake_minimum_required(VERSION 3.24.1)

project(ecs)

option(FETCH_LIBS OFF)

add_compile_options(-Wunused-function -Wall -Wextra -Wpedantic)

if (FETCH_LIBS)
    include(FetchContent)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(raylib)
else()
    find_package(raylib REQUIRED)
endif(FETCH_LIBS)
unset(FETCH_LIBS CACHE)

add_subdirectory(bin)
add_subdirectory(lib)
