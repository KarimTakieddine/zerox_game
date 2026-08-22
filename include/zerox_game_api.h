#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #define ZEROX_GAME_API __declspec(dllexport)
#else
    #if __GNUC__ >= 4
        #define ZEROX_GAME_API __attribute__ ((visibility ("default")))
    #else
        #define ZEROX_GAME_API
    #endif
#endif
