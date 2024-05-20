cmake_minimum_required(VERSION 3.5)
project(Apop VERSION 1.0)

set (CMAKE_CXX_STANDARD 17)

set(LIBS include/storage/memory.cpp)
add_library(libs STATIC "${LIBS}")

add_executable(Apop main.cpp)
target_link_libraries(Apop libs)
