file(GLOB_RECURSE PDF_RIPPER_CORE_TEST_SOURCES
    CONFIGURE_DEPENDS
    *.cpp
)

add_executable(pdf_ripper_core_tests
    ${PDF_RIPPER_CORE_TEST_SOURCES}
)

target_compile_features(pdf_ripper_core_tests PRIVATE cxx_std_23)

target_include_directories(pdf_ripper_core_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(pdf_ripper_core_tests
    PRIVATE
        pdf_ripper_core
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(pdf_ripper_core_tests)
