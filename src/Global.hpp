#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#define PROJECT_NAME            "DynamicFanControl"
#define MAJOR_VERSION           2
#define MINOR_VERSION           2

#ifdef USE_CMAKE_GENERATED
    #include "version_generated.hpp"
#else
    #define USE_GIT_INFO        0
#endif // #ifdef USE_CMAKE_GENERATED

#include "Types.hpp"

#endif // #ifndef GLOBAL_HPP
