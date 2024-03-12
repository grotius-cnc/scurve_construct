cmake_minimum_required(VERSION 3.5)

project(scurve_engine LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    main.cpp
    scurve_struct.h
    scurve_engine.h
    scurve_engine.cpp
    scurve_segment.h
    scurve_vector.h
    scurve_vector.cpp
)

add_executable(scurve_engine ${PROJECT_SOURCES} )

