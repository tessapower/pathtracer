cmake_minimum_required(VERSION 4.2)

project(pathtracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories for better organization
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#########################################################
# Find dependencies
#########################################################
find_package(Stb REQUIRED)
find_package(glm CONFIG REQUIRED)

#########################################################
# Collect source and header files
#########################################################
file(GLOB_RECURSE SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE HEADERS 
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

#########################################################
# Create executable
#########################################################
add_executable(pathtracer ${SOURCES} ${HEADERS})

#########################################################
# Include directories
#########################################################
target_include_directories(pathtracer 
    PRIVATE 
        "${PROJECT_SOURCE_DIR}/include"
        ${Stb_INCLUDE_DIR}
)

#########################################################
# Link libraries
#########################################################
target_link_libraries(pathtracer 
    PRIVATE 
        glm::glm-header-only
)

#########################################################
# Visual Studio specific settings
#########################################################
if(MSVC)
    # Organize files in Visual Studio solution explorer
    source_group(TREE "${PROJECT_SOURCE_DIR}/include" 
                 PREFIX "Header Files" 
                 FILES ${HEADERS})
    
    source_group(TREE "${PROJECT_SOURCE_DIR}/src" 
                 PREFIX "Source Files" 
                 FILES ${SOURCES})
    
    # Set Visual Studio startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                 PROPERTY VS_STARTUP_PROJECT pathtracer)
    
    # Enable multi-processor compilation
    target_compile_options(pathtracer PRIVATE /MP)
    
    # Set warning level
    target_compile_options(pathtracer PRIVATE /W4)
endif()

#########################################################
# Compiler-specific optimizations
#########################################################
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(pathtracer PRIVATE /O2 /Ob2)
    endif()
endif()
