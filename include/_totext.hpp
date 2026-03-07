// clang-format off
#include <_internal.hpp>
#include <_wingdi.hpp>
#include <_bitmap.hpp>
#include <_helpers.hpp>
// clang-format on

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
#include <optional>

// NOLINTBEGIN(cppcoreguidelines-narrowing-conversions)

constexpr long   CONSOLE_WIDTH { 140 };
constexpr double CONSOLE_WIDTHR { 140.0 };
constexpr double ONE { 1.000000000 };

namespace palettes {

    // characters in ascending order of luminance
    static constexpr char minimal[]  = { '_', '.', ',', '-', '=', '+', ':', ';', 'c', 'b', 'a', '!', '?', '1',
                                         '2', '3', '4', '5', '6', '7', '8', '9', '$', 'W', '#', '@', 'N' };

    static constexpr char base[]     = { ' ', '.', '-', ',', ':', '+', '~', ';', '(', '%', 'x', '1', '*', 'n', 'u',
                                         'T', '3', 'J', '5', '$', 'S', '4', 'F', 'P', 'G', 'O', 'V', 'X', 'E', 'Z',
                                         '8', 'A', 'U', 'D', 'H', 'K', 'W', '@', 'B', 'Q', '#', '0', 'M', 'N' };

    static constexpr char extended[] = { ' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'I', 'l',  '!', 'i', '>', '<', '~', '+', '_',
                                         '-', '?', ']',  '[', '}', '{', '1', ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n',
                                         'u', 'v', 'c',  'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q',  '0', 'O', 'Z', 'm', 'w', 'q', 'p',
                                         'd', 'b', 'k',  'h', 'a', 'o', '*', '#', 'M', 'W', '&',  '8', '%', 'B', '@', '$' };
}

template<unsigned long _length> static inline std::optional<std::string> to_raw_string(
    const bitmap& image, const char (&palette)[_length], unsigned (*const fnptr)(const RGBQUAD&) noexcept
) noexcept {
    if (image.height() < 0) {
        ::fprintf(stderr, "Error in %s, this function does not support bitmaps with top-down pixel ordering!\n", __PRETTY_FUNCTION__);
        return std::nullopt;
    }

    const long npixels = image.height() * image.width(); // total pixels in the image
    const long nchars  = npixels + image.height();       // space for an LF character '\n' to be appended to the end of each line

    std::string buffer {};
    buffer.resize(nchars + 1); // +1 is for the null terminator
    if (buffer.empty()) {
        ::fprintf(stderr, "Error in %s at line %d: call to std::string::resize failed!\n", __PRETTY_FUNCTION__, __LINE__);
        return std::nullopt;
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
            buffer[caret++] = internal::rgb::totext::maptochar(fnptr, &image.data()[nrows * image.width() + ncols], palette);
        // at the end of each scanline, append an LF
        buffer[caret++] = '\n';
    }

    buffer[caret] = 0; // null termination of the string

    assert(caret == nchars);
    return buffer;
}

template<unsigned long _length> static inline std::optional<std::string> to_downscaled_string(
    const bitmap& image, const char (&palette)[_length], unsigned (*const fnptr)(const RGBQUAD&) noexcept
) noexcept {
    if (image.height() < 0) {
        ::fprintf(stderr, "Error in %s, this function does not support bitmaps with top-down pixel ordering!\n", __PRETTY_FUNCTION__);
        return std::nullopt;
    }

    const long  block_d   = ::ceill(image.width() / CONSOLE_WIDTHR); // dimension of an individual square block
    const float blocksize = block_d * block_d;                       // number of pixels in a block - since our blocks are square

    long pblocksize_right = // number of pixels in each block in the rightmost column of incomplete blocks.
        // width of the image - (number of complete blocks * block dimension) will give the residual pixels along the horizontal axis
        // multiply that by block domension again, and we'll get the number of pixels in the incomplete block
        (image.width() - (image.width() / block_d) /* deliberate integer division to get only the count of complete blocks */ * block_d) *
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

    std::string buffer {};
    buffer.resize(nchars);
    if (buffer.empty()) {
        ::fprintf(stderr, "Error in %s at line %d: std::string::resize failed!\n", __PRETTY_FUNCTION__, __LINE__);
        return std::nullopt;
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

    unsigned long count {}; // NOLINT(readability-isolate-declaration)

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

            buffer[caret++] = internal::rgb::totext::maptochar(fnptr, { blockavg_blue, blockavg_green, blockavg_red }, palette);
            blockavg_blue = blockavg_green = blockavg_red = 0.000;
        }

        if (block_rows_end_with_incomplete_blocks) { // if there are partially filled blocks at the end of this row of blocks,

            for (long r = row; r > row - block_d; --r) {
                // shift the column delimiter backward by one block, to the end of the last complete block
                for (long c = col; c < image.width(); ++c) { // start from the end of the last complete block
                    offset          = (r * image.width()) + c;
                    blockavg_blue  += image.data()[offset].rgbBlue;
                    blockavg_green += image.data()[offset].rgbGreen;
                    blockavg_red   += image.data()[offset].rgbRed;
                }
            }

            assert(count == pblocksize_right); // fails

            blockavg_blue  /= pblocksize_right;
            blockavg_green /= pblocksize_right;
            blockavg_red   /= pblocksize_right;

            assert(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00);

            buffer[caret++] = internal::rgb::totext::maptochar(fnptr, { blockavg_blue, blockavg_green, blockavg_red }, palette);
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
                    blockavg_blue  += image.data()[offset].rgbBlue;
                    blockavg_green += image.data()[offset].rgbGreen;
                    blockavg_red   += image.data()[offset].rgbRed;
                }
            }

            blockavg_blue  /= pblocksize_bottom;
            blockavg_green /= pblocksize_bottom;
            blockavg_red   /= pblocksize_bottom;

            // if (!(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00))
            //     wprintf_s(L"Average (BGR) = (%.4f, %.4f, %.4f)\n", blockavg_blue, blockavg_green, blockavg_red);

            assert(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00);
            buffer[caret++] = internal::rgb::totext::maptochar(fnptr, { blockavg_blue, blockavg_green, blockavg_red }, palette);
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
template<unsigned long _length> static inline char* to_string(
    const bitmap& image, const char (&palette)[_length], unsigned (*const fnptr)(const RGBQUAD&) noexcept
) noexcept {
    if (image.width() <= CONSOLE_WIDTH) return to_raw_string(image, palette, fnptr);
    return to_downscaled_string(image, palette, fnptr);
}

#undef __INTERNAL

// NOLINTEND(cppcoreguidelines-narrowing-conversions)
