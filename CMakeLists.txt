cmake_minimum_required(VERSION 3.20)
project(DynamicFanControl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(ENABLE_CLANGD "Generate compile_commands.json for IDEs" OFF)

if(ENABLE_CLANGD)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

find_package(Boost REQUIRED NO_MODULE COMPONENTS log log_setup thread program_options)
find_package(yaml-cpp REQUIRED)

find_path(SENSORS_INCLUDE_DIR
    NAMES sensors/sensors.h)

find_library(SENSORS_LIBRARY
    NAMES sensors)

if (NOT SENSORS_INCLUDE_DIR OR NOT SENSORS_LIBRARY)
    message(FATAL_ERROR "lm-sensors not found! Please install libsensors-dev (or equivalent)")
endif()

add_subdirectory(src)
