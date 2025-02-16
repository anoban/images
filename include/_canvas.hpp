#pragma once
#define __INTERNAL
#if !defined(__CANVAS) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <immintrin.h>

#include <_bitmap.hpp>
#include <_cmaps.hpp>

class canvas final : public bitmap {
        // BELOW IS ABSOLUTELY CRITICAL NOT TO RUN INTO BUFFER OVERRUNS
        static constexpr unsigned long long FRACTAL_MAX_ITERATIONS { colourmaps::CMAPSIZE - 1 };
        static constexpr double             FRACTAL_EXPLODE_THRESHOLD { 4.0000 };

    public:
        canvas() noexcept = delete;

        explicit canvas(_In_ const wchar_t* const filename) noexcept : bitmap(filename) { }

        canvas(_In_ const long height, _In_ const long width) noexcept : bitmap { height, width } { }

        explicit canvas(_In_ const bitmap& image) noexcept : bitmap { image } { }

        explicit canvas(_In_ bitmap&& image) noexcept : bitmap { std::move(image) } { }

        canvas(_In_ const canvas& other) = default;

        canvas& operator=(_In_ const canvas& other) noexcept {
            if (this == std::addressof(other)) return *this;
            // since class canvas does not have any data members of its own, take
            // advantage of the base class's copy assignment operator
            bitmap::operator=(other);
            return *this;
        }

        // since class canvas does not have any data members of its own, take
        // advantage of the base class's move constructor
        canvas(_In_ canvas&& other) noexcept : bitmap(std::move(other)) { }

        canvas& operator=(_In_ canvas&& other) noexcept {
            if (this == std::addressof(other)) return *this;
            // again, leveraging the base class's move constructor
            bitmap::operator=(std::move(other));
            return *this;
        }

        ~canvas() noexcept = default;

        // resizes the pixel buffer and updates the metadata and remaps the old pixels
        // to the new buffer
        [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& resize(_In_ const long height, _In_ const long width) noexcept { }

        // this is a crude resize operation that just resizes the existing pixel
        // buffer and updates the metadata accordingly
        canvas& resize_for_overwrite(_In_ const long height, _In_ const long width) noexcept {
            // the members the need to be updated after a resize operation are [buffer],
            // [pixels], [buffer_size], file_header, info_header, file_size
            const auto newsize { sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * height * width };
            if (buffer_size < newsize) { // handle the reallocation
                auto* const temp_buffer { new (std::nothrow) unsigned char[newsize] };
                if (!temp_buffer) {
                    ::fputws(L"Error inside " __FUNCTIONW__ ", object is left in its original state\n", stderr);
                    return *this;
                }

                buffer      = temp_buffer;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                pixels      = reinterpret_cast<RGBQUAD*>(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
                buffer_size = newsize;
            }

            // keep the old buffer, pixels and buffer_size but update the others
            file_header.bfSize   = newsize;
            info_header.biHeight = height;
            info_header.biWidth  = width;
            file_size            = newsize;

            bitmap::metadata_to_buffer(); // sink the updated width and height to buffer
            return *this;
        }

        //----------------------------------------------------------------------------------------------------------------------------------//
        //                                              IMAGE TRANSFORMATION ROUTINES
        //                                              //
        //----------------------------------------------------------------------------------------------------------------------------------//

        // most methods that involve transformations of some sort return a reference
        // to self in order to facilitate method chaining
        canvas& fill_with(_In_ const RGBQUAD& pixel) noexcept {
            std::fill(begin(), end(), pixel);
            return *this;
        }

        // transform the image to black and white using the selected mechanism
        template<::BW_TRANSFORMATION method> canvas& to_blacknwhite() noexcept {
            switch (method) {
                case ::BW_TRANSFORMATION::AVERAGE : std::for_each(begin(), end(), internal::rgb::transformers::average); break;
                case ::BW_TRANSFORMATION::WEIGHTED_AVERAGE :
                    std::for_each(begin(), end(), internal::rgb::transformers::weighted_average);
                    break;
                case ::BW_TRANSFORMATION::LUMINOSITY : std::for_each(begin(), end(), internal::rgb::transformers::luminosity); break;
                case ::BW_TRANSFORMATION::BINARY     : std::for_each(begin(), end(), internal::rgb::transformers::binary); break;
            }
            return *this;
        }

        canvas& to_negative() noexcept {
            std::for_each(begin(), end(), internal::rgb::negative);
            return *this;
        }

        // remove the selected colours from the pixels of the image
        template<::RGB_TAG colour_combination> canvas& remove_colour() noexcept {
            std::for_each(begin(), end(), internal::rgb::removers::zero<colour_combination> {});
            return *this;
        }

        canvas& hflip() noexcept {
            // number of RGBQUAD structs a xmm register can hold
            static constexpr auto XMM_STORABLE_RGBQUADS { sizeof(__m128i) / sizeof(RGBQUAD) };

#if defined(__llvm__) && defined(__clang__)
            static constexpr __v16qu MASK { 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3 };
#elif defined(_MSC_VER) && defined(_MSC_FULL_VER)
            static constexpr __m128i MASK {
                // !!! WE WANT TO REVERSE THE PIXELS NOT THE BYTES, REVERSING PIXELS
                // MEANS REVERSING CONSECUTIVE 4 BYTE BLOCKS
                // !!! WE DO NOT WANT BYTES WITHIN EACH PIXELS REORDERED AS THIS WILL
                // CHANGE THE WAY COLOURS ARE INTERPRETED FROM PIXELS
                .m128i_u8 { 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3 }
            };
#endif
            __m128i    left {}, right {}; // NOLINT(readability-isolate-declaration)
            const long residues /* in RGBQUADs */ { static_cast<long>((width() / 2) % XMM_STORABLE_RGBQUADS) };
            RGBQUAD    temp {};
            long       col {}, row {}; // NOLINT(readability-isolate-declaration)

            // AVX2 OR AVX512 do not provide instructions to shuffle THE bytes across
            // the 64 byte zmm lane they only allow byte shuffling WITHIN 16 byte
            // boundaries WITHIN the zmm register, so going back to AVX1 :(

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (row = 0; row < height(); ++row) {
                for (col = 0; col < width() / 2 /* deliberate integer division */ - residues; col += XMM_STORABLE_RGBQUADS) {
                    left  = ::_mm_shuffle_epi8(::_mm_loadu_epi8(pixels + width() * row + col), MASK);
                    // !!! WE NEED sizeof(__m128i) BYTES BEFORE THE OFFSET width() * (row +
                    // 1) - col, NOT AFTER IT
                    right = ::_mm_shuffle_epi8(::_mm_loadu_epi8(pixels + width() * (row + 1) - col - XMM_STORABLE_RGBQUADS), MASK);

                    ::_mm_storeu_epi8(pixels + width() * (row + 1) - col - XMM_STORABLE_RGBQUADS, left);
                    ::_mm_storeu_epi8(pixels + width() * row + col, right);
                }

                // !!! HERE WE SWAP PIXELS NOT BYTES, SO NO NEED TO WORRY ABOUT THE PIXEL
                // BOUNDARIES
                for (; col < width() / 2; ++col) { // handle the residues
                    temp                              = pixels[width() * row + col];
                    pixels[width() * row + col]       = pixels[width() * (row + 1) - col];
                    pixels[width() * (row + 1) - col] = temp;
                }
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return *this;
        }

        canvas& vflip() noexcept { // reverse the order of scanlines in the image
            // number of RGBQUAD structs a zmm register can hold
            static constexpr auto ZMM_STORABLE_RGBQUADS { sizeof(__m512i) / sizeof(RGBQUAD) };
            __m512i               above {}, below {}; // NOLINT(readability-isolate-declaration)
            const long            residues /* in RGBQUADs */ { static_cast<long>(width() % ZMM_STORABLE_RGBQUADS) };
            RGBQUAD               temp {};
            long                  col {}, row {}; // NOLINT(readability-isolate-declaration)

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (row = 0; row < height() / 2 /* deliberate integer division */; ++row) {
                // we slice the height into half, if the height is an even number, cool,
                // we process all the rows, if it is an odd number we leave the middle one
                // untouced, no probss
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
            return *this;
        }

        //----------------------------------------------------------------------------------------------------------------------------------//
        //                                              PATTERN GENERATION ROUTINES //
        //----------------------------------------------------------------------------------------------------------------------------------//

        canvas& waves() noexcept {
            long double red {}, green {}, blue {}; // NOLINT(readability-isolate-declaration)

            for (long row = 0; row < height(); ++row) {
                for (long col = 0; col < width(); ++col) {
                    // minimum and maximum values returned by cos() are -1.000 and 1.000
                    // if cos() returned -1.000, red will be 1.000, if cos() returned 1.000,
                    // red will be 255.000, so all cool
                    red                                  = 128.0000 + 127.0000 * ::cos(0.02153 * row * col);
                    green                                = 128.0000 + 127.0000 * ::sin(0.01297 * row * col) - ::cos(0.1354 * col) / 2.0000;
                    blue                                 = 128.0000 + 127.0000 * ::cos(0.1248 * row) + ::sin(0.1573 * row * col) / 2.0000;
                    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    pixels[row * width() + col].rgbBlue  = static_cast<unsigned char>(blue);
                    pixels[row * width() + col].rgbGreen = static_cast<unsigned char>(green);
                    pixels[row * width() + col].rgbRed   = static_cast<unsigned char>(red);
                    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }
            return *this;
        }

        // most of Wikipdia's fractal implementations have a lot of show stopping
        // bugs, even though these are all inspired from Wikipedia's implementations,
        // these contain numerous refactors to fix those bugs
        // these fractal functions behave as if the scanlines in the pixel buffer are
        // ordered top-down, since these are fractals, this is not much of an issue
        // here

        // look up https://en.wikipedia.org/wiki/Julia_set
        [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& julia(
            _In_ const colourmaps::colourmap& cmap,
            _In_ const double&                escape_radius // choose escape_radius > 0 such that escape_radius**2
                                                            // - escape_radius >= sqrt(cx**2 + cy**2)
        ) noexcept {
            internal::complex<double> scaled_coordinates {}; // scaled between (-escape_radius, escape_radius)

            const double escrsq { escape_radius * escape_radius };

            unsigned long iterations {};
            double        xtemp {};

            // for each pixel in the image
            for (long row = 0; row < height(); ++row) {
                for (long col = 0; col < width(); ++col) {
                    // dividing col by width will scale the value (0, 1), multiplication by
                    // 2 shifts the scale to (0, 2) subtracting 1 shifts to (-1, 1) and the
                    // ultimate multiplication by escape_radius will shift it to
                    // (-escape_radius, escape_radius) same goes for the col too
                    scaled_coordinates.x() = (col / static_cast<double>(width()) * 2.000 - 1.0000) * escape_radius;
                    scaled_coordinates.y() = (row / static_cast<double>(height()) * 2.000 - 1.0000) * escape_radius;

                    iterations             = 0;

                    while (scaled_coordinates.x() * scaled_coordinates.x() + scaled_coordinates.y() * scaled_coordinates.y() < escrsq &&
                           ++iterations < FRACTAL_MAX_ITERATIONS) {
                        //
                        xtemp = scaled_coordinates.x() * scaled_coordinates.x() - scaled_coordinates.y() * scaled_coordinates.y();
                        scaled_coordinates = {};
                    }
                }
            }
        }

        // lookup https://en.wikipedia.org/wiki/Julia_set
        [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& multijulia(
            _In_ const colourmaps::colourmap& cmap,
            _In_ const double&                escape_radius // choose escape_radius > 0 such that escape_radius**2
                                                            // - escape_radius >= sqrt(cx**2 + cy**2)
        ) noexcept {
            internal::complex<double> scaled_coordinates {}; // scaled between (-escape_radius, escape_radius)

            const auto escaperadsq { escape_radius * escape_radius };

            unsigned long iterations {};
            double        xtemp {}, xsq {}, ysq {}; // NOLINT(readability-isolate-declaration)

            // for each pixel in the image
            for (long row = 0; row < height(); ++row) {
                for (long col = 0; col < width(); ++col) {
                    // dividing col by width will scale the value (0, 1), multiplication by
                    // 2 shifts the scale to (0, 2) subtracting 1 shifts to (-1, 1) and the
                    // ultimate multiplication by escape_radius will shift it to
                    // (-escape_radius, escape_radius) same goes for the col too
                    scaled_coordinates.x() = (col / static_cast<double>(width()) * 2.000 - 1.0000) * escape_radius;
                    scaled_coordinates.y() = (row / static_cast<double>(height()) * 2.000 - 1.0000) * escape_radius;

                    iterations             = 0;
                    xsq                    = scaled_coordinates.x() * scaled_coordinates.x();
                    ysq                    = scaled_coordinates.y() * scaled_coordinates.y();

                    while (xsq + ysq < escaperadsq && ++iterations < FRACTAL_MAX_ITERATIONS) {
                        //
                        xtemp              = xsq - ysq;
                        scaled_coordinates = {};
                    }
                }
            }
        }

        // look up https://en.wikipedia.org/wiki/Mandelbrot_set
        canvas& mandelbrot(_In_ const colourmaps::colourmap& cmap) noexcept {
            internal ::complex<double> scaled_coords {}, coords {}, squares {}; // NOLINT(readability-isolate-declaration)
            double                     xtemp {};
            unsigned long long         niterations {};

            // for each pixel in the image
            for (long row = 0; row < height(); ++row) {
                for (long col = 0; col < width(); ++col) {
                    // Mandelbrot space is defined by a horizontal range (-2.00, 0.47) and a
                    // vertical range (-1.12, 1.12) dividing col by width gives a value (0,
                    // 1), multiplying that by 2.4700 upscales to to (0, 2.4700), then
                    // subtracting 2.000 gives us (-2.000, 0.4700)
                    scaled_coords.x() = col / static_cast<double>(width()) * 2.47000 - 2.0000;
                    // dividing row by height will give a value (0, 1), subtracting 0.500
                    // shifts it to (-0.5, 0.5) then by multiplying by 2.2400 we could
                    // stretch it to (-1.1200, 1.1200)
                    scaled_coords.y() = (row / static_cast<double>(height()) - 0.5000) * 2.24000;

                    // Mandelbrot equation :: Z = Z^2 + C
                    // throughout the loop Z is iteratively updated while C is updated only
                    // once before this loop begins at some point in the loop Z may undergo
                    // an explosion, which is defined as the sums of squares of Z's x and y
                    // coordinates surpassing a predefined threshold value the selection of
                    // colour down the road depends on whcih happens first, whether the loop
                    // reaching maximum number of iterations or the explosion
                    while ( // BEWARE OF POTENTIAL SHORT CIRCUITS BELOW, REORDERING THE
                        // SUBEXPRESSIONS MAY LEAD TO UNWANTED RUNTIME BEHAVIOURS
                        ++niterations < FRACTAL_MAX_ITERATIONS &&
                        (squares.x() = coords.x() * coords.x()) + (squares.y() = coords.y() * coords.y()) /* magnitude of Z */
                            < FRACTAL_EXPLODE_THRESHOLD
                    ) {
                        xtemp      = squares.x() - squares.y() + scaled_coords.x();
                        // IT IS ABSOULUTELY CRITICAL THAT WHEN THE IMAGINARY PART (y) IS
                        // UPDATED, THE USED REAL PART (x) SHOULD BE IN THE ORIGINAL
                        // (PREVIOUS) STATE
                        coords.y() = 2.000 * coords.x() * coords.y() + scaled_coords.y();
                        coords.x() = xtemp; // THUS UPDATING THE REAL PART (x) AFTER THE
                                            // UPDATE OF IMAGINARY PART (y)
                    }

                    pixels[row * width() + col] = cmap.at(niterations); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    niterations                 = 0;
                    coords.x() = coords.y() = 0.0000;
                }
            }
            return *this;
        }

        // look up https://en.wikipedia.org/wiki/Tricorn_(mathematics)
        [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& tricorn(_In_ const colourmaps::colourmap& cmap) noexcept {
            // NOLINTNEXTLINE(readability-isolate-declaration)
            internal ::complex<double> scaled_coords {}, coords {}, squares {}; // x (-2.5, 1) y (-1, 1)
            double                     xtemp {};
            unsigned long long         niterations {};

            // for each pixel in the image
            for (long row = 0; row < height(); ++row) {
                for (long col = 0; col < width(); ++col) {
                    // dividing col by width gives a value (0, 1), multiplying that by 3.500
                    // upscales to to (0, 3.5), then subtracting 2.5 gives us (-2.5, 1)
                    scaled_coords.x() = col / static_cast<double>(width()) * 3.5000 - 2.5000;
                    // dividing row by height will give a value (0, 1), by multiplying by 2
                    // we could stretch it to (0, 2), then by subtracting 1, we could shift
                    // the range to (-1, 1)
                    scaled_coords.y() = row / static_cast<double>(height()) * 2.0000 - 1.0000;

                    while (++niterations < FRACTAL_MAX_ITERATIONS &&
                           (squares.x() = coords.x() * coords.x()) + (squares.y() = coords.y() * coords.y()) < FRACTAL_EXPLODE_THRESHOLD) {
                        xtemp      = squares.x() - squares.y() + scaled_coords.x();
                        coords.y() = -2.000 * coords.x() * coords.y() + scaled_coords.y();
                        coords.x() = xtemp;
                    }

                    pixels[row * width() + col] = cmap.at(niterations); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    niterations                 = 0;
                    squares.x() = squares.y() = 0.0000;
                }
            }
            return *this;
        }

        template<ANGLES> [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& rotate() noexcept;

        template<> [[deprecated("IMPLEMENTATION INCOMPLETE")]] canvas& rotate<ANGLES::NINETY>() noexcept { }

        // returns a deep copy of self
        [[nodiscard]] canvas copy() const noexcept { return *this; }

        [[nodiscard]] bitmap unwrap() const noexcept {
            static_assert(sizeof(bitmap) == sizeof(canvas));
            return *this; // a non-destructive object slicing happens here
        }
};

#undef __INTERNAL
