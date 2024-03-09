cmake_minimum_required(VERSION 3.5)

project(scurve_construct LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    main.cpp
    scurve_struct.h
    scurve_construct.h
    scurve_construct.cpp
)

add_executable(scurve_construct ${PROJECT_SOURCES} )

