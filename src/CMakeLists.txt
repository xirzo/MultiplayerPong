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

add_library(properties STATIC properties.c properties.h)

add_library(sound STATIC sound.c sound.h)

add_executable(${PROJECT_NAME} main_client.c)

target_link_libraries(${PROJECT_NAME} PUBLIC flecs raylib input movement render aabb utils score server properties sound)

if (NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_executable(pong_server main_server.c)
target_include_directories(pong_server PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pong_server PUBLIC server properties)
