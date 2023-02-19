cmake_minimum_required(VERSION 3.15)

set(TARGET_NAME "service-discovery")

project(service-discovery-utility VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(examples)
add_subdirectory(Tests)
add_subdirectory(rest-api)