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

struct PNGCHUNK {
        uint16_t               LENGTH {};     // number of bytes in the DATA field.
        std::array<uint8_t, 4> CHUNKNAME {};  // an array of 4 chars
        std::vector<uint8_t>   DATA {};       // chunk data
        uint32_t               CHUNKCRC32 {}; // a 32 bit Cyclic Redundancy Check (CRC) value calculated from the data.
};

#endif                                        // !__PNG_H_