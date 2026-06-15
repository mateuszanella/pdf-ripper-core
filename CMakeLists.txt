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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Build parameters
# ------------------------------------------------------------------------------
option(PDF_RIPPER_CORE_TIDY_INCLUDE_TESTS "Include tests in clang-tidy analysis" OFF)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(PDF_RIPPER_CORE_ENABLE_TESTS "Build pdf_ripper_core tests" ON)
else()
    option(PDF_RIPPER_CORE_ENABLE_TESTS "Build pdf_ripper_core tests" OFF)
endif()

if(PDF_RIPPER_CORE_ENABLE_TESTS)
    include(CTest)
endif()

set(PDF_RIPPER_CORE_IO_CORE_GIT_REPOSITORY
    "https://github.com/ripper-org/io-core.git"
    CACHE STRING
    "Git repository used when fetching io-core"
)

set(PDF_RIPPER_CORE_IO_CORE_GIT_TAG
    "main"
    CACHE STRING
    "Git ref (tag/branch/commit) used when fetching io-core"
)

# ------------------------------------------------------------------------------
# First-party dependencies (internal sibling projects)
# ------------------------------------------------------------------------------
include(FetchContent)

message(STATUS "Fetching io-core from git: ${PDF_RIPPER_CORE_IO_CORE_GIT_REPOSITORY} (${PDF_RIPPER_CORE_IO_CORE_GIT_TAG})")
FetchContent_Declare(
    io_ripper_core
    GIT_REPOSITORY ${PDF_RIPPER_CORE_IO_CORE_GIT_REPOSITORY}
    GIT_TAG        ${PDF_RIPPER_CORE_IO_CORE_GIT_TAG}
)
FetchContent_MakeAvailable(io_ripper_core)

# ------------------------------------------------------------------------------
# Third-party dependencies
# ------------------------------------------------------------------------------
FetchContent_Declare(
    zlib-ng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        2.3.2
)

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

file(GLOB_RECURSE PDF_RIPPER_CORE_HEADERS
    CONFIGURE_DEPENDS
    include/**/*.hpp
)

file(GLOB_RECURSE PDF_RIPPER_CORE_TEST_FILES
    CONFIGURE_DEPENDS
    tests/*.cpp
    tests/*.hpp
)

set(PDF_RIPPER_CORE_FORMAT_FILES
    ${PDF_RIPPER_CORE_HEADERS}
    ${PDF_RIPPER_CORE_SOURCES}
    ${PDF_RIPPER_CORE_TEST_FILES}
)

set(PDF_RIPPER_CORE_TIDY_FILES ${PDF_RIPPER_CORE_SOURCES})
if(PDF_RIPPER_CORE_ENABLE_TESTS AND PDF_RIPPER_CORE_TIDY_INCLUDE_TESTS)
    list(APPEND PDF_RIPPER_CORE_TIDY_FILES ${PDF_RIPPER_CORE_TEST_FILES})
endif()

find_program(PDF_RIPPER_CORE_CLANG_FORMAT_BIN NAMES clang-format)
if(PDF_RIPPER_CORE_CLANG_FORMAT_BIN)
    add_custom_target(pdf_ripper_core_format
        COMMAND ${PDF_RIPPER_CORE_CLANG_FORMAT_BIN} -i ${PDF_RIPPER_CORE_FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting pdf-core source files with clang-format"
        VERBATIM
    )

    add_custom_target(pdf_ripper_core_format_check
        COMMAND ${PDF_RIPPER_CORE_CLANG_FORMAT_BIN} --dry-run --Werror ${PDF_RIPPER_CORE_FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking pdf-core source formatting with clang-format"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found; 'pdf_ripper_core_format' and 'pdf_ripper_core_format_check' targets will fail until installed.")

    add_custom_target(pdf_ripper_core_format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )

    add_custom_target(pdf_ripper_core_format_check
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

find_program(PDF_RIPPER_CORE_CLANG_TIDY_BIN NAMES clang-tidy)
if(PDF_RIPPER_CORE_CLANG_TIDY_BIN)
    add_custom_target(pdf_ripper_core_tidy
        COMMAND ${PDF_RIPPER_CORE_CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR} ${PDF_RIPPER_CORE_TIDY_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running pdf-core clang-tidy static analysis"
        VERBATIM
    )
else()
    message(STATUS "clang-tidy not found; 'pdf_ripper_core_tidy' target will fail until installed.")

    add_custom_target(pdf_ripper_core_tidy
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

# Reader/writer implementations are provided by io-core.

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
target_link_libraries(pdf_ripper_core
    PUBLIC
        zlib-ng
        io_ripper_core::io_ripper_core
)

if(PDF_RIPPER_CORE_ENABLE_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )

    FetchContent_MakeAvailable(Catch2)

    set(BUILD_TESTING ON CACHE BOOL "Enable testing" FORCE)
    add_subdirectory(tests)
endif()
