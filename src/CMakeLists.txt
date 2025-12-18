# Find all source files
file(GLOB SOURCES *.c *.cpp)

# Find all header files
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")

# Add CMakeLists to list of source files
list(APPEND
    SOURCES
    "CMakeLists.txt"
)

# Add source files to the executable
target_sources(pathtracer PRIVATE ${SOURCES} ${HEADERS})
