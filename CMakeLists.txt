
cmake_minimum_required(VERSION 3.5)

project(scurve_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(scurve_jog_plot)
add_subdirectory(scurve_auto_plot)
add_subdirectory(scurve_engine)

