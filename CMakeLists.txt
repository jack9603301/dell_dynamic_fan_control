cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" CODE_VERSION_RAW)
string(STRIP ${CODE_VERSION_RAW} CODE_VERSION)
project(DynamicFanControl VERSION ${CODE_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(ENABLE_CLANGD "Generate compile_commands.json for IDEs" OFF)
option(ENABLE_SYSTEMD "Enable Systemd support" ON)


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

if (ENABLE_SYSTEMD)
    set(APP_INSTALL_BINARY "${CMAKE_INSTALL_FULL_BINDIR}/dynamic_fan_control")
    set(APP_INSTALL_CONFIG "${CMAKE_INSTALL_FULL_SYSCONFDIR}/sysconfig/fan_config.yaml"
        CACHE FILEPATH "Default configuration file path for DynamicFanControl") 

    configure_file(
        ${CMAKE_SOURCE_DIR}/conf/DynamicFanControl.service.in
        ${CMAKE_BINARY_DIR}/generated/conf/DynamicFanControl.service
        @ONLY)

    install(FILES ${CMAKE_BINARY_DIR}/generated/conf/DynamicFanControl.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system)
    set(APP_SYSTEMD_SERVICE "${CMAKE_INSTALL_FULL_LIBDIR}/systemd/system/DynamicFanControl.service")

    message(STATUS "Systemd Service Config File: ${APP_SYSTEMD_SERVICE}")
    message(STATUS "Default Config File: ${APP_INSTALL_CONFIG}")
    message(STATUS "You need to create the configuration file yourself.")
endif()
