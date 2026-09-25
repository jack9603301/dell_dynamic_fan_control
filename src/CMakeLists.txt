file(GLOB_RECURSE SOURCES
    *.cpp
    **/*.cpp
    *.hpp
    **/*.hpp
)

add_executable(dynamic_fan_control ${SOURCES})

target_include_directories(dynamic_fan_control PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${yaml-cpp_INCLUDE_DIRS}
    ${SENSORS_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(dynamic_fan_control PRIVATE
    Boost::log
    Boost::log_setup
    Boost::program_options
    yaml-cpp::yaml-cpp
    ${SENSORS_LIBRARY}
)

find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_SHORT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    message(STATUS "Git Hash: ${GIT_HASH}")
    
    target_compile_definitions(dynamic_fan_control PRIVATE USE_GIT_INFO=1)
else()
    target_compile_definitions(dynamic_fan_control PRIVATE USE_GIT_INFO=0)
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.hpp.in
    ${CMAKE_BINARY_DIR}/generated/Version_Generated.hpp
    @ONLY
)

target_compile_definitions(dynamic_fan_control PRIVATE USE_CMAKE_GENERATED)

install(TARGETS dynamic_fan_control RUNTIME DESTINATION bin)
