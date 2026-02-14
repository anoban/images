// clang-format off
#include <internal.hpp>
#include <_windef.hpp>
#include <_bitmap.hpp>
// clang-format on

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

// NOLINTBEGIN(readability-redundant-inline-specifier,modernize-avoid-c-arrays)

namespace _to_text_impl {

    constexpr auto CONSOLE_WIDTH { 140LLU };
    constexpr auto CONSOLE_WIDTHR { 140.0 };
    constexpr auto ONE { 1.000000000F };

    // characters in ascending order of luminance
    static constexpr char palette_minimal[]  = { '_', '.', ',', '-', '=', '+', ':', ';', 'c', 'b', 'a', '!', '?', '1',
                                                 '2', '3', '4', '5', '6', '7', '8', '9', '$', 'W', '#', '@', 'N' };

    static constexpr char palette[]          = { ' ', '.', '-', ',', ':', '+', '~', ';', '(', '%', 'x', '1', '*', 'n', 'u',
                                                 'T', '3', 'J', '5', '$', 'S', '4', 'F', 'P', 'G', 'O', 'V', 'X', 'E', 'Z',
                                                 '8', 'A', 'U', 'D', 'H', 'K', 'W', '@', 'B', 'Q', '#', '0', 'M', 'N' };

    static constexpr char palette_extended[] = { ' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'I', 'l',  '!', 'i', '>', '<', '~', '+', '_',
                                                 '-', '?', ']',  '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n',
                                                 'u', 'v', 'c',  'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q',  '0', 'O', 'Z', 'm', 'w', 'q', 'p',
                                                 'd', 'b', 'k',  'h', 'a', 'o', '*', '#', 'M', 'W', '&',  '8', '%', 'B', '@', '$' };

    namespace _rgbops {

        // arithmetic average of an RGB pixel values
        static constexpr inline unsigned __attribute__((__always_inline__)) arithmetic_average(const RGBQUAD& pixel) noexcept {
            // we don't want overflows or truncations here
            return (static_cast<unsigned>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.000;
        }

        // weighted average of an RGB pixel values
        static constexpr inline unsigned __attribute__((__always_inline__)) weighted_average(const RGBQUAD& pixel) noexcept {
            return pixel.rgbBlue * 0.299 + pixel.rgbGreen * 0.587 + pixel.rgbRed * 0.114;
        }

        // average of minimum and maximum RGB values in a pixel
        static constexpr inline unsigned __attribute__((__always_inline__)) minmax_average(const RGBQUAD& pixel) noexcept {
            // we don't want overflows or truncations here
            return (static_cast<float>(std::min({ pixel.rgbBlue, pixel.rgbGreen, pixel.rgbRed })) +
                    std::max({ pixel.rgbBlue, pixel.rgbGreen, pixel.rgbRed })) /
                   2.0000;
        }

        // luminosity of an RGB pixel
        static constexpr inline unsigned __attribute__((__always_inline__)) luminosity(const RGBQUAD& pixel) noexcept {
            return pixel.rgbBlue * 0.2126 + pixel.rgbGreen * 0.7152 + pixel.rgbRed * 0.0722;
        }

    } // namespace _rgbops

    namespace _pixeltochar {

        // taking it for granted that the input will never be a negative value,
        static constexpr inline unsigned __attribute__((__always_inline__)) __nudge(const float& _value) noexcept {
            assert(_value > 0.00000);
            return _value < 1.000000 ? 1 : static_cast<unsigned>(_value);
        }

        template<typename _TyPixOpFunc, unsigned long _length> static constexpr inline __attribute__((__always_inline__))
        typename std::enable_if<std::is_same<_TyPixOpFunc, unsigned (*)(const RGBQUAD&) noexcept>::value, char>::type
        maptochar(const _TyPixOpFunc _function, const RGBQUAD& _pixel, const char (&_palette)[_length]) noexcept {
            const unsigned offset = _function(_pixel);
            return _palette[offset ? __nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * _length) - 1 : 0];
        }

    } // namespace _pixeltochar

    template<long unsigned _length> static inline char* to_raw_string(
        const bitmap& image, const char (&palette)[_length], unsigned (*const fnptr)(const RGBQUAD&) noexcept
    ) noexcept {
        if (image.height() < 0) {
            ::fputs("Error in to_raw_string, this tool does not support bitmaps with top-down pixel ordering!\n", stderr);
            return nullptr;
        }

        const long npixels = image.height() * image.width(); // total pixels in the image
        // 1 char for each pixel + 2 additional wchar_ts for CRLF at the end of each scanline
        // space for two extra characters ('\r', '\n') to be appended to the end of each line
        const long nchars  = npixels + 2 * image.height();

        char* const buffer = new (std::nothrow) char[nchars + 1]; // +1 is for the nullptr terminator
        if (!buffer) {
            ::fprintf(stderr, "Error in %s at line %d: memory allocation failed!\n", __PRETTY_FUNCTION__, __LINE__);
            return nullptr;
        }

        // pixels are organized in rows from bottom to top and, within each row, from left to right, each row is called a "scan line".
        // if the image height is given as a negative number, then the rows are ordered from top to bottom (in most contemporary .BMP images, the pixel ordering seems to be bottom up)
        // (pixel at the top left corner of the image)
        //                                            10 11 12 13 14 15 16 17 18 19 <-- this will be the last pixel (pixel at the bottom right corner of the image)
        //                                            .............................
        // this is the first pixel in the buffer -->  00 01 02 03 04 05 06 07 08 09
        // (pixel at the top left corner of the image)

        long caret {};
        // presuming pixels are ordered bottom up, start the traversal from the last pixel and move up.
        // traverse up along the height, for each row, starting with the last row,
        for (long nrows = image.height() - 1LL; nrows >= 0; --nrows) {
            // traverse left to right inside "scan lines"
            for (long ncols = 0; ncols < image.width(); ++ncols) // for each pixel in the row,
                buffer[caret++] = _pixeltochar::maptochar(fnptr, &image.data()[nrows * image.width() + ncols], palette);
            // at the end of each scanline, append a CRLF!
            buffer[caret++] = '\n';
            buffer[caret++] = '\r';
        }

        buffer[caret] = 0; // null termination of the string

        assert(caret == nchars);
        return buffer;
    }

    static inline char* to_downscaled_string(const bitmap& image) noexcept {
        if (image.height() < 0) {
            ::fprintf(stderr, "Error in %s, bitmaps with top-down pixel ordering are not supported!\n", __PRETTY_FUNCTION__);
            return nullptr;
        }

        const unsigned long block_d /* dimension of an individual square block */ = ::ceill(image.width() / CONSOLE_WIDTHR);

        const float blocksize /* number of pixels in a block */                   = block_d * block_d; // since our blocks are square

        long pblocksize_right = // number of pixels in each block in the rightmost column of incomplete blocks.
            // width of the image - (number of complete blocks * block dimension) will give the residual pixels along the horizontal axis
            // multiply that by block domension again, and we'll get the number of pixels in the incomplete block
            (image.width() -
             (image.width() / block_d) /* deliberate integer division to get only the count of complete blocks */ * block_d) *
            block_d;
        assert(pblocksize_right < blocksize);

        // the block size to represent the number of pixels held by the last row blocks
        long pblocksize_bottom = (image.height() - (image.height() / block_d) * block_d) * block_d;
        assert(pblocksize_bottom < blocksize);

        const long nblocks_w = ::ceill(image.width() / (float) block_d);
        const long nblocks_h = ::ceill(image.height() / (float) block_d);

        // we have to compute the average R, G & B values for all pixels inside each pixel blocks and use the average to represent
        // that block as a char. one char in our buffer will have to represent (block_w x block_h) number of RGBQUADs
        const long nchars    = nblocks_h * (nblocks_w + 2) + 1; // saving two wide chars for CRLF!, the +1 is for the nullptr terminator

        char* const buffer   = new (std::nothrow) char[nchars];
        if (!buffer) {
            ::fprintf(stderr, "Error in %s at line %d: memory allocation failed!\n", __PRETTY_FUNCTION__, __LINE__);
            return nullptr;
        }

        // NOLINTBEGIN(readability-isolate-declaration)
        // per block averages of the rgbBlue, rgbGreen and rgbRed values
        float      blockavg_blue {}, blockavg_green {}, blockavg_red {};
        long       caret {}, offset {}, col {}, row {};
        // true if the image width is not divisible by 140 without remainders
        const bool block_rows_end_with_incomplete_blocks    = image.width() % CONSOLE_WIDTH;
        // true if the image height is not divisible by block_d without remainders
        const bool block_columns_end_with_incomplete_blocks = image.height() % block_d;
        // NOLINTEND(readability-isolate-declaration)

        unsigned long full = 0, incomplete = 0, count = 0; // NOLINT(readability-isolate-declaration)

        // row = image.height() will get us to the last pixel of the first (last in the buffer) scanline with (r * image.width())
        // hence, row = image.height() - 1 so we can traverse pixels in the first scanline with (r * image.width()) + c
        for (row = image.height() - 1; row >= (block_d - 1); row -= block_d) { // start the traversal at the bottom most scan line
                                                                               // wprintf_s(L"row = %lld\n", row);
            for (col = 0; col <= image.width() - block_d; col += block_d) {    // traverse left to right in scan lines
                // wprintf_s(L"row = %lld, col = %lld\n", row, col);

                for (long r = row; r > row - block_d; --r) { // deal with blocks
                    for (long c = col; c < col + block_d; ++c) {
                        offset          = (r * image.width()) + c;
                        blockavg_blue  += image.data()[offset].rgbBlue;
                        blockavg_green += image.data()[offset].rgbGreen;
                        blockavg_red   += image.data()[offset].rgbRed;
                    }
                }

                assert(count == block_d * block_d);

                blockavg_blue  /= blocksize;
                blockavg_green /= blocksize;
                blockavg_red   /= blocksize;

                assert(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00);

                buffer[caret++] = blockmap(blockavg_blue, blockavg_green, blockavg_red);
                blockavg_blue = blockavg_green = blockavg_red = 0.000;
            }

            if (block_rows_end_with_incomplete_blocks) { // if there are partially filled blocks at the end of this row of blocks,

                for (long r = row; r > row - block_d; --r) {
                    // shift the column delimiter backward by one block, to the end of the last complete block
                    for (long c = col; c < image.width(); ++c) { // start from the end of the last complete block
                        offset          = (r * image.width()) + c;
                        blockavg_blue  += image->_pixels[offset].rgbBlue;
                        blockavg_green += image->_pixels[offset].rgbGreen;
                        blockavg_red   += image->_pixels[offset].rgbRed;
                    }
                }

                assert(count == pblocksize_right); // fails

                blockavg_blue  /= pblocksize_right;
                blockavg_green /= pblocksize_right;
                blockavg_red   /= pblocksize_right;

                assert(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00);

                buffer[caret++] = blockmap(blockavg_blue, blockavg_green, blockavg_red);
                blockavg_blue = blockavg_green = blockavg_red = 0.000; // reset the block averages
            }

            buffer[caret++] = '\n';
            buffer[caret++] = '\r';
        }

        assert(row < block_d);

        if (block_columns_end_with_incomplete_blocks) { // process the last incomplete row of pixel blocks here,

            for (col = 0; col < image.width(); col += block_d) { // col must be 0 at the start of this loop

                for (long r = row; r >= 0; --r) {                // r delimits the start row of the block being defined
                    for (long c = col; c < col + block_d; ++c) { // c delimits the start column of the block being defined
                        offset          = (r * image.width()) + c;
                        blockavg_blue  += image->_pixels[offset].rgbBlue;
                        blockavg_green += image->_pixels[offset].rgbGreen;
                        blockavg_red   += image->_pixels[offset].rgbRed;
                    }
                }

                blockavg_blue  /= pblocksize_bottom;
                blockavg_green /= pblocksize_bottom;
                blockavg_red   /= pblocksize_bottom;

                // if (!(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00))
                //     wprintf_s(L"Average (BGR) = (%.4f, %.4f, %.4f)\n", blockavg_blue, blockavg_green, blockavg_red);

                assert(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00);
                buffer[caret++] = blockmap(blockavg_blue, blockavg_green, blockavg_red);
                blockavg_blue = blockavg_green = blockavg_red = 0.000; // reset the block averages
            }

            buffer[caret++] = '\n';
            buffer[caret++] = '\r';
        }

        buffer[caret++] = 0; // using the last byte as null terminator

        // now caret == nwchars, so an attempt to write at caret will now raise an access violation exception or a heap corruption error

        assert(caret == nchars);

        return buffer;
    }

    // an image width predicated dispatcher for to_raw_string and to_downscaled_string
    static inline char* to_string(const bitmap_t* const image) {
        if (image.width() <= CONSOLE_WIDTH) return to_raw_string(image);
        return to_downscaled_string(image);
    }
} // namespace _to_text_impl

// NOLINTEND(readability-redundant-inline-specifier,modernize-avoid-c-arrays)
