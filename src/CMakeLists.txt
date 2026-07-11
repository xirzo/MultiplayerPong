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

add_library(network_systems STATIC network_systems.c network_systems.h)
target_link_libraries(network_systems PUBLIC flecs input movement properties server)

add_library(systems STATIC systems.c systems.h)
target_link_libraries(systems PUBLIC flecs aabb input movement network_systems render score server sound)

add_library(entities STATIC entities.c entities.h)
target_link_libraries(entities PUBLIC flecs input movement properties render score sound)

add_library(network STATIC network.c network.h)
target_link_libraries(network PUBLIC movement properties server)

add_library(menus STATIC menus.c menus.h)

add_library(components STATIC components.c components.h)
target_link_libraries(components PUBLIC flecs)

add_executable(${PROJECT_NAME} main_client.c)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    menus
    flecs 
    components
    raylib 
    input 
    movement 
    render 
    aabb 
    utils 
    score 
    server 
    properties 
    sound 
    systems 
    entities 
    network 
    network_systems
)

if (NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_executable(pong_server main_server.c)
target_include_directories(pong_server PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pong_server PUBLIC server properties)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )

    install(TARGETS ${PROJECT_NAME} DESTINATION .)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION .)
    install(TARGETS pong_server DESTINATION .)
endif()
