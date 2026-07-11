cmake_minimum_required(VERSION 3.24.1)

project(pong_client)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FETCH_LIBS OFF)

add_compile_options(-Wunused-function -Wall -Wextra -Wpedantic)

if (FETCH_LIBS)
    include(FetchContent)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(raylib)
else()
    find_package(raylib REQUIRED)
endif(FETCH_LIBS)
unset(FETCH_LIBS CACHE)

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_ARCH)
    if(SYSTEM_ARCH STREQUAL "amd64")
        set(SYSTEM_ARCH "x86_64")
    endif()

    string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_OS)

    set(CPACK_PACKAGE_FILE_NAME "${SYSTEM_ARCH}_${SYSTEM_OS}_pong")
    
    set(CPACK_GENERATOR "ZIP")
    include(CPack)
endif()
