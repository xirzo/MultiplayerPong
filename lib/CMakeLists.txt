add_library(flecs STATIC flecs.c flecs.h)

add_library(aabb STATIC aabb.c aabb.h)
target_link_libraries(aabb PUBLIC movement flecs)

if (NOT MSVC)
    target_link_libraries(aabb PUBLIC m)
endif()

add_library(input STATIC input.c input.h)
target_link_libraries(input PUBLIC raylib)

add_library(movement STATIC movement.c movement.h)
target_link_libraries(movement PUBLIC flecs utils)

add_library(render STATIC render.c render.h)
target_link_libraries(render PUBLIC raylib flecs)

add_library(utils STATIC utils.c utils.h)

add_library(score STATIC score.c score.h)
target_link_libraries(score PUBLIC flecs movement)

add_library(server STATIC server.c server.h)
