cmake_minimum_required(VERSION 4.0)

project(pathtracer
    VERSION 0.1.0
    LANGUAGES CXX
)

#########################################################
# Project Options
#########################################################
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmark suite" OFF)  # Disabled for now
option(ENABLE_PIX "Enable PIX profiling markers" ON)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

#########################################################
# C++ Standard
#########################################################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#########################################################
# Output Directories
#########################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-configuration output directories
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

#########################################################
# Windows SDK Requirements
#########################################################
if(WIN32)
    # Require Windows 10 SDK version 10.0.22000.0 or later for DX12
    if(CMAKE_SYSTEM_VERSION)
        message(STATUS "Windows SDK Version: ${CMAKE_SYSTEM_VERSION}")
    endif()
endif()

#########################################################
# Find Dependencies
#########################################################
# Stb is only needed for texture loading (Phase 10)
find_package(Stb QUIET)
if(Stb_FOUND)
    message(STATUS "Stb found - texture loading enabled")
else()
    message(STATUS "Stb not found - texture loading disabled (not needed yet)")
endif()

find_package(glm CONFIG QUIET)
if(glm_FOUND)
    message(STATUS "glm found - vector/matrix math enabled")
else()
    message(STATUS "glm not found - vector/matrix math disabled (not needed yet)")
endif()

# Optional: PIX for profiling
if(ENABLE_PIX)
    find_package(WinPixEventRuntime CONFIG)
    if(WinPixEventRuntime_FOUND)
        message(STATUS "PIX Event Runtime found - profiling enabled")
    else()
        message(WARNING "PIX Event Runtime not found - profiling disabled")
        set(ENABLE_PIX OFF)
    endif()
endif()

# Optional: Google Test
if(BUILD_TESTS)
    find_package(GTest CONFIG)
    if(NOT GTest_FOUND)
        message(WARNING "Google Test not found - tests disabled")
        set(BUILD_TESTS OFF)
    endif()
endif()

#########################################################
# Source Files (Auto-discovery with CONFIGURE_DEPENDS)
#########################################################
# CONFIGURE_DEPENDS tells CMake to check for new/removed files on each build
# This is the modern approach that combines convenience with reliability

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

# Headers (for IDE organization)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

#########################################################
# Shader Compilation
#########################################################
set(SHADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# Find Windows SDK and DXC (DirectX Shader Compiler)
if(WIN32)
    # Determine Windows SDK directory
    if(DEFINED ENV{WindowsSdkDir})
        set(WINDOWS_SDK_DIR "$ENV{WindowsSdkDir}")
    else()
        # Common installation paths
        set(WINDOWS_SDK_DIR "C:/Program Files (x86)/Windows Kits/10")
        if(NOT EXISTS "${WINDOWS_SDK_DIR}")
            set(WINDOWS_SDK_DIR "C:/Program Files/Windows Kits/10")
        endif()
    endif()

    # Find all available SDK versions
    if(EXISTS "${WINDOWS_SDK_DIR}/bin")
        file(GLOB SDK_VERSIONS LIST_DIRECTORIES true "${WINDOWS_SDK_DIR}/bin/10.*")
        if(SDK_VERSIONS)
            # Sort to get the latest version
            list(SORT SDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)
            list(GET SDK_VERSIONS 0 LATEST_SDK_BIN)
            get_filename_component(WINDOWS_SDK_VERSION ${LATEST_SDK_BIN} NAME)
            message(STATUS "Found Windows SDK version: ${WINDOWS_SDK_VERSION}")
        endif()
    endif()

    # Build search paths for DXC
    set(DXC_SEARCH_PATHS)

    # From environment variables (Developer Command Prompt)
    if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
        list(APPEND DXC_SEARCH_PATHS "$ENV{WindowsSdkDir}bin/$ENV{WindowsSDKVersion}x64")
    endif()

    # From CMake's detected SDK version
    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        list(APPEND DXC_SEARCH_PATHS "${WINDOWS_SDK_DIR}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
    endif()

    # From our detected latest SDK version
    if(WINDOWS_SDK_VERSION)
        list(APPEND DXC_SEARCH_PATHS "${WINDOWS_SDK_DIR}/bin/${WINDOWS_SDK_VERSION}/x64")
    endif()

    # Fallback: search all SDK versions
    if(SDK_VERSIONS)
        foreach(SDK_PATH ${SDK_VERSIONS})
            list(APPEND DXC_SEARCH_PATHS "${SDK_PATH}/x64")
        endforeach()
    endif()

    # Find DXC
    find_program(DXC_EXECUTABLE
        NAMES dxc.exe dxc
        PATHS ${DXC_SEARCH_PATHS}
        DOC "DirectX Shader Compiler (dxc.exe)"
        NO_DEFAULT_PATH
    )

    # If not found, try system PATH
    if(NOT DXC_EXECUTABLE)
        find_program(DXC_EXECUTABLE
            NAMES dxc.exe dxc
            DOC "DirectX Shader Compiler (dxc.exe)"
        )
    endif()
endif()

if(DXC_EXECUTABLE)
    message(STATUS "Found DXC: ${DXC_EXECUTABLE}")
else()
    message(WARNING "DXC not found - shaders will not be compiled")
endif()

# Shader files (auto-discover all HLSL files)
file(GLOB SHADER_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/shaders/*.hlsl"
)

# Custom command to compile shaders
if(DXC_EXECUTABLE)
    foreach(SHADER ${SHADER_FILES})
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        # SHADER already contains full path from GLOB
        set(SHADER_INPUT "${SHADER}")
        set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.cso")

        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${DXC_EXECUTABLE}
                -T cs_6_0                    # Compute shader model 6.0
                -E CSMain                    # Entry point
                -Fo ${SHADER_OUTPUT}         # Output file
                -Zi                          # Debug info
                -Qembed_debug                # Embed debug info
                -Od                          # Disable optimizations (Debug)
                $<$<CONFIG:Release>:-O3>     # Max optimizations (Release)
                ${SHADER_INPUT}
            DEPENDS ${SHADER_INPUT}
            COMMENT "Compiling shader: ${SHADER_NAME}"
            VERBATIM
        )

        list(APPEND COMPILED_SHADERS ${SHADER_OUTPUT})
    endforeach()

    # Custom target for shader compilation
    add_custom_target(CompileShaders ALL
        DEPENDS ${COMPILED_SHADERS}
    )

    # Copy compiled shaders to output directory for each configuration
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        add_custom_command(TARGET CompileShaders POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/bin/${CONFIG}/shaders"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${COMPILED_SHADERS}
                "${CMAKE_BINARY_DIR}/bin/${CONFIG}/shaders/"
        )
    endforeach()

    add_custom_command(TARGET CompileShaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Shaders copied to all output directories"
    )
endif()

#########################################################
# Main Executable
#########################################################
add_executable(pathtracer ${ALL_SOURCES} ${HEADERS})

# Add shader files to target (for IDE visibility)
if(SHADER_FILES)
    target_sources(pathtracer PRIVATE ${SHADER_FILES})
    # Disable automatic shader compilation by VS (we use custom DXC commands)
    set_source_files_properties(${SHADER_FILES} PROPERTIES
        VS_TOOL_OVERRIDE "None"
        HEADER_FILE_ONLY TRUE
    )
endif()

# Precompiled headers
target_precompile_headers(pathtracer PRIVATE "${PROJECT_SOURCE_DIR}/include/stdafx.h")

# Shader dependency
if(DXC_EXECUTABLE)
    add_dependencies(pathtracer CompileShaders)
endif()

#########################################################
# Include Directories
#########################################################
target_include_directories(pathtracer
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        $<$<BOOL:${Stb_FOUND}>:${Stb_INCLUDE_DIR}>
)

#########################################################
# Link Libraries
#########################################################
target_link_libraries(pathtracer
    PRIVATE
        # DirectX 12 libraries
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
)

# Optional dependencies
if(glm_FOUND)
    target_link_libraries(pathtracer PRIVATE glm::glm-header-only)
endif()

# PIX Event Runtime
if(ENABLE_PIX AND WinPixEventRuntime_FOUND)
    target_link_libraries(pathtracer PRIVATE WinPixEventRuntime::WinPixEventRuntime)
    target_compile_definitions(pathtracer PRIVATE USE_PIX=1)
endif()

#########################################################
# Compile Definitions
#########################################################
target_compile_definitions(pathtracer PRIVATE
    # Windows version targeting
    WINVER=0x0A00          # Windows 10
    _WIN32_WINNT=0x0A00    # Windows 10

    # Unicode support
    UNICODE
    _UNICODE

    # Debug/Release specific
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:DEBUG_BUILD=1>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:RELEASE_BUILD=1>
)

#########################################################
# Compiler Flags - MSVC
#########################################################
if(MSVC)
    target_compile_options(pathtracer PRIVATE
        # Multi-processor compilation
        /MP

        # Warning level
        /W4

        # External header warnings (Windows SDK, WRL)
        /external:W0                 # No warnings from external headers
        /external:anglebrackets      # Treat #include <...> as external

        # Treat warnings as errors (only for our code)
        /WX

        # Additional warnings
        /w14242  # Conversion, possible loss of data
        /w14254  # Operator conversion
        /w14263  # Member function doesn't override
        /w14265  # Class has virtual functions but no virtual destructor
        /w14287  # Unsigned/negative constant mismatch
        /w14296  # Expression is always true/false
        /w14311  # Pointer truncation
        /w14545  # Expression before comma has no effect
        /w14546  # Function call before comma missing argument list
        /w14547  # Operator has no effect
        /w14549  # Operator has no effect
        /w14555  # Expression has no effect
        /w14619  # Unknown warning number
        /w14640  # Thread-safe static initialization
        /w14826  # Conversion is sign-extended
        /w14905  # Wide string literal cast
        /w14906  # String literal cast
        /w14928  # Illegal copy-initialization

        # Disable specific warnings
        /wd4068  # Unknown pragma (for non-MSVC compilers)
        /wd4265  # Class has virtual functions but non-virtual destructor (WRL headers)

        # Debug configuration
        $<$<CONFIG:Debug>:/Od>      # No optimization
        $<$<CONFIG:Debug>:/Zi>      # Debug info
        $<$<CONFIG:Debug>:/RTC1>    # Runtime checks

        # Release configuration
        $<$<CONFIG:Release>:/O2>    # Maximize speed
        $<$<CONFIG:Release>:/Ob2>   # Inline expansion
        $<$<CONFIG:Release>:/Oi>    # Intrinsic functions
        $<$<CONFIG:Release>:/Ot>    # Favor fast code
        $<$<CONFIG:Release>:/GL>    # Whole program optimization

        # Profile configuration
        $<$<CONFIG:RelWithDebInfo>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/Zi>
        $<$<CONFIG:RelWithDebInfo>:/PROFILE>
    )

    # Linker flags
    target_link_options(pathtracer PRIVATE
        # Windows subsystem (for WinMain entry point)
        /SUBSYSTEM:WINDOWS

        # Release link-time optimizations
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>

        # Profile configuration
        $<$<CONFIG:RelWithDebInfo>:/PROFILE>

        # Debug configuration
        $<$<CONFIG:Debug>:/DEBUG:FULL>
    )

    # Static analysis
    if(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(pathtracer PRIVATE /analyze)
    endif()

    # Address Sanitizer
    if(ENABLE_ASAN)
        target_compile_options(pathtracer PRIVATE /fsanitize=address)
        target_link_options(pathtracer PRIVATE /INCREMENTAL:NO)
    endif()

    # Visual Studio solution organization
    source_group(TREE "${PROJECT_SOURCE_DIR}/include"
                 PREFIX "Header Files"
                 FILES ${HEADERS})

    source_group(TREE "${PROJECT_SOURCE_DIR}/src"
                 PREFIX "Source Files"
                 FILES ${ALL_SOURCES})

    # Group shader files in a "Shaders" folder in VS
    if(SHADER_FILES)
        source_group("Shaders" FILES ${SHADER_FILES})
    endif()

    # Set startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 PROPERTY VS_STARTUP_PROJECT pathtracer)

    # Note: VS_DEBUGGER_WORKING_DIRECTORY is not set here to avoid $<CONFIG> issues
    # Visual Studio will use the executable's directory by default
    # which is correct since shaders are copied to bin/$<CONFIG>/shaders/
endif()

#########################################################
# Benchmark Executable
#########################################################
if(BUILD_BENCHMARKS)
    # Gather tool sources
    file(GLOB BENCHMARK_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/tools/*.cpp"
    )

    # Create a copy of ALL_SOURCES and remove main.cpp
    set(LIB_SOURCES ${ALL_SOURCES})
    list(FILTER LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

    add_executable(pathtracer-benchmark
        ${BENCHMARK_SOURCES}
        ${LIB_SOURCES}
    )

    # Precompiled headers
    target_precompile_headers(pathtracer-benchmark PRIVATE "${PROJECT_SOURCE_DIR}/include/stdafx.h")

    target_include_directories(pathtracer-benchmark
        PRIVATE
            "${PROJECT_SOURCE_DIR}/include"
            $<$<BOOL:${Stb_FOUND}>:${Stb_INCLUDE_DIR}>
    )

    target_link_libraries(pathtracer-benchmark
        PRIVATE
            d3d12.lib
            dxgi.lib
            dxguid.lib
            d3dcompiler.lib
    )

    if(glm_FOUND)
        target_link_libraries(pathtracer-benchmark PRIVATE glm::glm-header-only)
    endif()

    target_compile_definitions(pathtracer-benchmark PRIVATE
        BENCHMARK_MODE=1
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        UNICODE
        _UNICODE
    )

    # Copy MSVC settings
    if(MSVC)
        target_compile_options(pathtracer-benchmark PRIVATE /MP /W4 /wd4265)
        target_link_options(pathtracer-benchmark PRIVATE /SUBSYSTEM:WINDOWS)
        # Note: VS_DEBUGGER_WORKING_DIRECTORY is not set to avoid $<CONFIG> issues
    endif()
endif()

#########################################################
# Unit Tests
#########################################################
if(BUILD_TESTS AND GTest_FOUND)
    enable_testing()

    # Gather all test sources
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/tests/unit/*.cpp"
        "${PROJECT_SOURCE_DIR}/tests/integration/*.cpp"
    )

    # Create a copy of ALL_SOURCES and remove main.cpp
    set(LIB_SOURCES ${ALL_SOURCES})
    list(FILTER LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

    add_executable(pathtracer-tests
        ${TEST_SOURCES}
        ${LIB_SOURCES}
    )

    # Precompiled headers
    target_precompile_headers(pathtracer-tests PRIVATE "${PROJECT_SOURCE_DIR}/include/stdafx.h")

    target_include_directories(pathtracer-tests
        PRIVATE
            "${PROJECT_SOURCE_DIR}/include"
            $<$<BOOL:${Stb_FOUND}>:${Stb_INCLUDE_DIR}>
    )

    target_link_libraries(pathtracer-tests
        PRIVATE
            GTest::gtest
            GTest::gtest_main
    )

    if(glm_FOUND)
        target_link_libraries(pathtracer-tests PRIVATE glm::glm-header-only)
    endif()

    target_compile_definitions(pathtracer-tests PRIVATE
        UNICODE
        _UNICODE
    )

    # Discover tests
    include(GoogleTest)
    gtest_discover_tests(pathtracer-tests)

    if(MSVC)
        target_compile_options(pathtracer-tests PRIVATE /MP /W4)
    endif()
endif()

#########################################################
# Installation
#########################################################
install(TARGETS pathtracer
    RUNTIME DESTINATION bin
)

if(DXC_EXECUTABLE)
    install(FILES ${COMPILED_SHADERS}
        DESTINATION bin/shaders
    )
endif()

#########################################################
# Status Messages
#########################################################
message(STATUS "")
message(STATUS "=== PathTracer Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Build Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "PIX Profiling: ${ENABLE_PIX}")
message(STATUS "Address Sanitizer: ${ENABLE_ASAN}")
message(STATUS "Shader Compiler: ${DXC_EXECUTABLE}")
message(STATUS "================================")
message(STATUS "")
