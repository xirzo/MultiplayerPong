add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PUBLIC flecs raylib input movement render aabb utils score server properties)

if (NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib)

add_executable(pong_server server.c)
target_include_directories(pong_server PUBLIC ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(pong_server PUBLIC server properties)
