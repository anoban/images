#pragma once

#ifndef __PNG_H_
    #define __PNG_H_

    #include <algorithm>
    #include <array>
    #include <cassert>
    #include <cstdint>
    #include <format>
    #include <limits>
    #include <optional>
    #include <stdexcept>
    #include <string>
    #include <vector>

    #ifdef _WIN32
        #define _AMD64_ // architecture
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
    #endif

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>

#endif // !__PNG_H_