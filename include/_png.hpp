#pragma once
#define __INTERNAL
#if !defined(__IMAGEIO) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

// https://www.w3.org/TR/png-3/
class png final {
    public:
        static constexpr unsigned long long PNG_CHUNK_NAME_LENGTH { 4 };

        class png_chunk final {
                unsigned       length;
                char           name[PNG_CHUNK_NAME_LENGTH];
                unsigned char* data;
                unsigned       crc;
        };

    private:

    public:
};

#undef __INTERNAL
