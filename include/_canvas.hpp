#pragma once
#define __INTERNAL
#if !defined(__CANVAS) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <immintrin.h>

#include <_bitmap.hpp>
#include <_cmaps.hpp>

class canvas final : public bitmap {
    public:
        canvas() noexcept = delete;

        explicit canvas(_In_ const wchar_t* const filename) noexcept : bitmap(filename) { }

        canvas(_In_ const long height, _In_ const long width) noexcept : bitmap { height, width } { }

        explicit canvas(_In_ const bitmap& image) noexcept : bitmap { image } { }

        explicit canvas(_In_ bitmap&& image) noexcept : bitmap { std::move(image) } { }

        canvas(_In_ const canvas& other) = default;

        canvas& operator=(_In_ const canvas& other) noexcept { }

        canvas(_In_ canvas&& other) noexcept : bitmap(std::move(other)) { }

        canvas& operator=(_In_ canvas&& other) noexcept { }

        ~canvas() noexcept = default;

        void                                         fill(_In_ const RGBQUAD& pixel) noexcept { std::fill(begin(), end(), pixel); }

        // transform the image to black and white using the selected mechanism
        template<rgb::BW_TRANSFORMATION method> void to_blacknwhite() noexcept {
            switch (method) {
                case rgb::BW_TRANSFORMATION::AVERAGE          : std::for_each(begin(), end(), rgb::transformers::average); break;
                case rgb::BW_TRANSFORMATION::WEIGHTED_AVERAGE : std::for_each(begin(), end(), rgb::transformers::weighted_average); break;
                case rgb::BW_TRANSFORMATION::LUMINOSITY       : std::for_each(begin(), end(), rgb::transformers::luminosity); break;
                case rgb::BW_TRANSFORMATION::BINARY           : std::for_each(begin(), end(), rgb::transformers::binary); break;
            }
        }

        // remove the selected colours from the pixels of the image
        template<rgb::RGB_TAG colour_combination> void remove_colour() noexcept {
            std::for_each(begin(), end(), rgb::removers::zero<colour_combination> {});
        }

        // INCOMPLETE AND BUGGY
        void hflip() noexcept { // number of RGBQUAD structs a zmm register can hold
            static constexpr auto    ZMM_STORABLE_RGBQUADS { sizeof(__m512i) / sizeof(RGBQUAD) };
            static constexpr __m512i BYTE_REVERSE_MASK {};
            __m512i                  left {}, right {}; // NOLINT(readability-isolate-declaration)
            const long               residues /* in RGBQUADs */ { static_cast<long>(width() % ZMM_STORABLE_RGBQUADS) };
            RGBQUAD                  temp {};
            long                     col {}, row {}; // NOLINT(readability-isolate-declaration)

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (row = 0; row < height(); ++row) {
                for (col = 0; col < width() / 2 /* deliberate integer division */ - residues; col += ZMM_STORABLE_RGBQUADS) {
                    left = ::_mm512_loadu_epi8(pixels + width() * row + col);
                    ::_mm512_permutex2var_epi8(left, BYTE_REVERSE_MASK, left);
                    right = ::_mm512_loadu_epi8(pixels + width() * (height() - row - 1) + col);
                    ::_mm512_storeu_epi8(pixels + width() * row + col, right);
                    ::_mm512_storeu_epi8(pixels + width() * (height() - row - 1) + col, left);
                }

                for (; col < width(); ++col) { // handle the residues
                    temp                                         = pixels[width() * row + col];
                    pixels[width() * row + col]                  = pixels[width() * (height() - row - 1) + col];
                    pixels[width() * (height() - row - 1) + col] = temp;
                }
            }
        }

        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic) }

        void vflip() noexcept { // reverse the order of scanlines in the image
            // number of RGBQUAD structs a zmm register can hold
            static constexpr auto ZMM_STORABLE_RGBQUADS { sizeof(__m512i) / sizeof(RGBQUAD) };
            __m512i               above {}, below {}; // NOLINT(readability-isolate-declaration)
            const long            residues /* in RGBQUADs */ { static_cast<long>(width() % ZMM_STORABLE_RGBQUADS) };
            RGBQUAD               temp {};
            long                  col {}, row {}; // NOLINT(readability-isolate-declaration)

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (row = 0; row < height() / 2 /* deliberate integer division */; ++row) {
                // we slice the height into half, if the height is an even number, cool, we process all the rows,
                // if it is an odd number we leave the middle one untouced, no probss
                for (col = 0; col < width() - residues; col += ZMM_STORABLE_RGBQUADS) { // keep swapping the scanlines
                    above = ::_mm512_loadu_epi8(pixels + width() * row + col);
                    below = ::_mm512_loadu_epi8(pixels + width() * (height() - row - 1) + col);
                    ::_mm512_storeu_epi8(pixels + width() * row + col, below);
                    ::_mm512_storeu_epi8(pixels + width() * (height() - row - 1) + col, above);
                }

                for (; col < width(); ++col) { // handle the residues
                    temp                                         = pixels[width() * row + col];
                    pixels[width() * row + col]                  = pixels[width() * (height() - row - 1) + col];
                    pixels[width() * (height() - row - 1) + col] = temp;
                }
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        [[nodiscard]] canvas copy() const noexcept { return *this; }

        [[nodiscard]] bitmap unwrap() const noexcept {
            static_assert(sizeof(bitmap) == sizeof(canvas));
            return *this; // a non-destructive object slicing happens here
        }
};

#undef __INTERNAL
