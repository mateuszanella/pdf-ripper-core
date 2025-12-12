cmake_minimum_required(VERSION 3.20)

project(pdf_ripper_core
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDF_RIPPER_BUILD_EXAMPLES "Build example executables" ON)

add_library(pdf_ripper_core
    src/Core/PDF.cpp
)

target_compile_features(pdf_ripper_core PUBLIC cxx_std_23)

target_include_directories(pdf_ripper_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(PDF_RIPPER_BUILD_EXAMPLES)
    add_executable(test
        src/main.cpp
    )
    target_link_libraries(test PRIVATE pdf_ripper_core)
endif()
