add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PUBLIC flecs raylib input movement render aabb utils score)

if (NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib)

add_executable(pong_server server.c)
add_executable(pong_client client.c)
