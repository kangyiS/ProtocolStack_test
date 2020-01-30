cmake_minimum_required(VERSION 2.8)

add_compile_options(-std=c++11)

project(protocol_stack)

add_subdirectory(src/transport_layer)
add_subdirectory(src/network_layer)
add_subdirectory(src/datalink_layer)
add_subdirectory(src/global_func)
add_subdirectory(src/network_base)

find_package(Threads)

include_directories(include)

aux_source_directory(. ROOT_SRC)

add_executable(demo ${ROOT_SRC})

target_link_libraries(demo TRANSPORT_LAYER NETWORK_LAYER DATALINK_LAYER NETWORK_BASE GLOBAL ${CMAKE_THREAD_LIBS_INIT})
