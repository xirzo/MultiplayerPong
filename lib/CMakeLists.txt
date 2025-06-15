add_library(flecs STATIC flecs.c flecs.h)

add_library(input STATIC input.c input.h)

target_link_libraries(input PUBLIC raylib)
