project(qni-core)

cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")

add_subdirectory(api)
add_subdirectory(include)
add_subdirectory(src)
