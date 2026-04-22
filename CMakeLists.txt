cmake_minimum_required(VERSION 3.20)

# ------------------------------------------------------------------------------
# Project configuration
# ------------------------------------------------------------------------------
project(pdf_ripper_core
    VERSION 0.1.0
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# Language standard
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Build parameters
# ------------------------------------------------------------------------------
option(BUILD_EXAMPLES "Build example executables" ON)

# ------------------------------------------------------------------------------
# External dependencies
# ------------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    zlib-ng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        2.3.2
)

# ------------------------------------------------------------------------------
# External dependency configuration
# ------------------------------------------------------------------------------
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_COMPAT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(zlib-ng)

# ------------------------------------------------------------------------------
# Library sources
# ------------------------------------------------------------------------------
file(GLOB_RECURSE PDF_RIPPER_CORE_SOURCES
    CONFIGURE_DEPENDS
    src/core/*.cpp
)

# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------
add_library(pdf_ripper_core
    ${PDF_RIPPER_CORE_SOURCES}
)

target_compile_features(pdf_ripper_core PUBLIC cxx_std_23)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(pdf_ripper_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ------------------------------------------------------------------------------
# Linking
# ------------------------------------------------------------------------------
target_link_libraries(pdf_ripper_core PUBLIC zlib-ng)

# ------------------------------------------------------------------------------
# Examples
# ------------------------------------------------------------------------------
if(BUILD_EXAMPLES)
    add_executable(pdf_ripper_core_example_read_document
        examples/read_document.cpp
    )
    target_link_libraries(pdf_ripper_core_example_read_document PRIVATE pdf_ripper_core)
endif()
