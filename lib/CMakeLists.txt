add_library(flecs STATIC flecs.c flecs.h)

add_library(aabb STATIC aabb.c aabb.h)
target_link_libraries(aabb PUBLIC flecs)

add_library(input STATIC input.c input.h)
target_link_libraries(input PUBLIC raylib)

add_library(movement STATIC movement.c movement.h)
target_link_libraries(movement PUBLIC flecs)

add_library(render STATIC render.c render.h)
target_link_libraries(render PUBLIC raylib flecs)
