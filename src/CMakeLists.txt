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
)

target_link_libraries(dynamic_fan_control PRIVATE
    Boost::log
    Boost::log_setup
    Boost::program_options
    yaml-cpp::yaml-cpp
    ${SENSORS_LIBRARY}
)

install(TARGETS dynamic_fan_control RUNTIME DESTINATION bin)
