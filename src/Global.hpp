#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#define PROJECT_NAME                    "DynamicFanControl"
#define STOP_TOKEN_WAITFOR_MSTIMEOUT      100

#ifdef USE_CMAKE_GENERATED
    #include "Version_Generated.hpp"
#else
    #define USE_GIT_INFO        0
#endif // #ifdef USE_CMAKE_GENERATED

#include "Types.hpp"

#endif // #ifndef GLOBAL_HPP
