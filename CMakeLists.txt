cmake_minimum_required(VERSION 4.2)
project(pathtracer)

set(CMAKE_CXX_STANDARD 23)

# Find dependencies
find_package(Stb REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(pathtracer src/main.cpp)

# Link GLM
target_link_libraries(pathtracer PRIVATE glm::glm-header-only)

# Include stb headers
target_include_directories(pathtracer PRIVATE ${Stb_INCLUDE_DIR})
