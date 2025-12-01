cmake_minimum_required(VERSION 3.28)
project(pathtracer)

set(CMAKE_CXX_STANDARD 23)

find_package(Stb REQUIRED)

add_executable(pathtracer src/main.cpp)

target_include_directories(pathtracer PRIVATE ${Stb_INCLUDE_DIR})
