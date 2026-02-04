cmake_minimum_required(VERSION 3.20)

project(pdf_ripper_core
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDF_RIPPER_BUILD_EXAMPLES "Build example executables" ON)

include(FetchContent)

FetchContent_Declare(
    zlib-ng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        2.3.2
)

set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_COMPAT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(zlib-ng)

file(GLOB_RECURSE PDF_RIPPER_CORE_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

add_library(pdf_ripper_core
    ${PDF_RIPPER_CORE_SOURCES}
)

target_compile_features(pdf_ripper_core PUBLIC cxx_std_23)

target_include_directories(pdf_ripper_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(pdf_ripper_core PUBLIC zlib-ng)

if(PDF_RIPPER_BUILD_EXAMPLES)
    add_executable(test
        src/main.cpp
    )
    target_link_libraries(test PRIVATE pdf_ripper_core)
endif()
