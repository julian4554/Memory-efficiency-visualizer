cmake_minimum_required(VERSION 3.22)

add_executable(unit_tests
        test_measure.cpp
        test_measure_runner.cpp
        doctest.h

)

# Produktivcode in die Tests linken
target_sources(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core/measure.c
        ${CMAKE_SOURCE_DIR}/src/core/measure_runner.cpp
)

# Header-Pfade
target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        # doctest.h
)
