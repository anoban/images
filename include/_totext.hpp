// clang-format off
#include <internal.hpp>
#include <_windef.hpp>
// clang-format on

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

// NOLINTBEGIN(readability-redundant-inline-specifier,modernize-avoid-c-arrays)

namespace _to_text_impl {

    constexpr auto CONSOLE_WIDTH { 140LL };
    constexpr auto CONSOLE_WIDTHR { 140.0 };
    constexpr auto ONE { 1.000000000F };

    // characters in ascending order of luminance
    static constexpr char palette_minimal[]  = { '_', '.', ',', '-', '=', '+', ':', ';', 'c', 'b', 'a', '!', '?', '1',
                                                 '2', '3', '4', '5', '6', '7', '8', '9', '$', 'W', '#', '@', 'N' };

    static constexpr char palette[]          = { ' ', '.', '-', ',', ':', '+', '~', ';', '(', '%', 'x', '1', '*', 'n', 'u',
                                                 'T', '3', 'J', '5', '$', 'S', '4', 'F', 'P', 'G', 'O', 'V', 'X', 'E', 'Z',
                                                 '8', 'A', 'U', 'D', 'H', 'K', 'W', '@', 'B', 'Q', '#', '0', 'M', 'N' };

    static constexpr char palette_extended[] = {
        ' ', '.', '\'', '`', '^',  '"', ',', ':', ';', 'I', 'l', '!', 'i', '>', '<', '~', '+', '_', '-', '?', ']', '[', '}', '{',
        '1', ')', '(',  '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 'c', 'z', 'X', 'Y', 'U', 'J', 'C',  '',  'Q', '0',
        'O', 'Z', 'm',  'w', 'q',  'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&', '8', '%', 'B', '@', '$'
    };

    namespace _pixel_ops {

        // arithmetic average of an RGB pixel values
        static constexpr inline unsigned arithmetic_average(const wingdi::RGBQUAD& pixel) noexcept {
            // we don't want overflows or truncations here
            return (static_cast<float>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.000;
        }

        // weighted average of an RGB pixel values
        static constexpr inline unsigned weighted_average(const wingdi::RGBQUAD& pixel) noexcept {
            return pixel.rgbBlue * 0.299 + pixel.rgbGreen * 0.587 + pixel.rgbRed * 0.114;
        }

        // average of minimum and maximum RGB values in a pixel
        static constexpr inline unsigned minmax_average(const wingdi::RGBQUAD& pixel) noexcept {
            // we don't want overflows or truncations here
            return (static_cast<float>(std::min({ pixel.rgbBlue, pixel.rgbGreen, pixel.rgbRed })) +
                    std::max({ pixel.rgbBlue, pixel.rgbGreen, pixel.rgbRed })) /
                   2.0000;
        }

        // luminosity of an RGB pixel
        static constexpr inline unsigned luminosity(const wingdi::RGBQUAD& pixel) noexcept {
            return pixel.rgbBlue * 0.2126 + pixel.rgbGreen * 0.7152 + pixel.rgbRed * 0.0722;
        }

    } // namespace _pixel_ops

    // taking it for granted that the input will never be a negative value,
    static inline unsigned nudge(const float _value) { return _value < 1.000000 ? 1 : _value; }

    static inline wchar_t arithmetic_mapper(const wingdi::RGBQUAD& pixel, const wchar_t* const palette, const unsigned plength) {
        const unsigned offset = (((float) (pixel.rgbBlue)) + pixel.rgbGreen + pixel.rgbRed) / 3.000; // can range from 0 to 255
        // hence, offset / (float)(std::numeric_limits<unsigned char>::max()) can range from 0.0 to 1.0
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t weighted_mapper(const wingdi::RGBQUAD& pixel, const wchar_t* const palette, const unsigned plength) {
        const unsigned offset = pixel.rgbBlue * 0.299 + pixel.rgbGreen * 0.587 + pixel.rgbRed * 0.114;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t minmax_mapper(const wingdi::RGBQUAD& pixel, const wchar_t* const palette, const unsigned plength) {
        const unsigned offset = (((float) (std::min(std::min(pixel.rgbBlue, pixel.rgbGreen), pixel.rgbRed))) +
                                 (std::max(std::max(pixel.rgbBlue, pixel.rgbGreen), pixel.rgbRed))) /
                                2.0000;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t luminosity_mapper(const wingdi::RGBQUAD& pixel, const wchar_t* const palette, const unsigned plength) {
        const unsigned offset = pixel.rgbBlue * 0.2126 + pixel.rgbGreen * 0.7152 + pixel.rgbRed * 0.0722;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t arithmetic_blockmapper(
        const float rgbBlue, const float rgbGreen, const float rgbRed, const wchar_t* const palette, const unsigned plength
    ) {
        const unsigned offset = (rgbBlue + rgbGreen + rgbRed) / 3.000; // can range from 0 to 255
        // hence, offset / (float)(std::numeric_limits<unsigned char>::max()) can range from 0.0 to 1.0
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t weighted_blockmapper(
        const float rgbBlue, const float rgbGreen, const float rgbRed, const wchar_t* const palette, const unsigned plength
    ) {
        const unsigned offset = rgbBlue * 0.299 + rgbGreen * 0.587 + rgbRed * 0.114;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t minmax_blockmapper(
        const float rgbBlue, const float rgbGreen, const float rgbRed, const wchar_t* const palette, const unsigned plength
    ) {
        const unsigned offset = (std::min(std::min(rgbBlue, rgbGreen), rgbRed) + std::max(std::max(rgbBlue, rgbGreen), rgbRed)) / 2.0000;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t luminosity_blockmapper(
        const float rgbBlue, const float rgbGreen, const float rgbRed, const wchar_t* const palette, const unsigned plength
    ) {
        const unsigned offset = rgbBlue * 0.2126 + rgbGreen * 0.7152 + rgbRed * 0.0722;
        return palette[offset ? nudge(offset / (float) (std::numeric_limits<unsigned char>::max()) * plength) - 1 : 0];
    }

    static inline wchar_t* __cdecl to_raw_string(const bitmap_t* const image) noexcept {
        if (image->_infoheader.biHeight < 0) {
            ::fputs("Error in to_raw_string, this tool does not support bitmaps with top-down pixel ordering!\n", stderr);
            return NULL;
        }

        const int64_t npixels = (int64_t) image->_infoheader.biHeight * image->_infoheader.biWidth; // total pixels in the image
        const int64_t nwchars /* 1 wchar_t for each pixel + 2 additional wchar_ts for CRLF at the end of each scanline */ =
            npixels + 2LLU * image->_infoheader.biHeight;
        // space for two extra wchar_ts ('\r', '\n') to be appended to the end of each line

        wchar_t* const buffer = malloc((nwchars + 1) * sizeof(wchar_t)); // and the +1 is for the NULL terminator
        if (!buffer) {
            fprintf(stderr, L"Error in %s @ line %d: malloc failed!\n", __FUNCTIONW__, __LINE__);
            return NULL;
        }

        // pixels are organized in rows from bottom to top and, within each row, from left to right, each row is called a "scan line".
        // if the image height is given as a negative number, then the rows are ordered from top to bottom (in most contemporary .BMP images, the pixel ordering seems to be bottom up)
        // (pixel at the top left corner of the image)
        //                                            10 11 12 13 14 15 16 17 18 19 <-- this will be the last pixel (pixel at the bottom right corner of the image)
        //                                            .............................
        // this is the first pixel in the buffer -->  00 01 02 03 04 05 06 07 08 09
        // (pixel at the top left corner of the image)

        int64_t caret = 0;
        // presuming pixels are ordered bottom up, start the traversal from the last pixel and move up.
        // traverse up along the height, for each row, starting with the last row,
        for (int64_t nrows = image->_infoheader.biHeight - 1LL; nrows >= 0; --nrows) {
            // traverse left to right inside "scan lines"
            for (int64_t ncols = 0; ncols < image->_infoheader.biWidth; ++ncols) // for each pixel in the row,
                buffer[caret++] = map(&image->_pixels[nrows * image->_infoheader.biWidth + ncols]);
            // at the end of each scanline, append a CRLF!
            buffer[caret++] = '\n';
            buffer[caret++] = '\r';
        }

        buffer[caret] = 0; // null termination of the string

        assert(caret == nwchars);
        return buffer;
    }

    static inline wchar_t* __cdecl to_downscaled_string(const bitmap_t* const image) noexcept {
        if (image->_infoheader.biHeight < 0) {
            fputws(L"Error in to_downscaled_string, this tool does not support bitmaps with top-down pixel ordering!\n", stderr);
            return NULL;
        }

        const int64_t block_d /* dimension of an individual square block */ = ceill(image->_infoheader.biWidth / CONSOLE_WIDTHR);

        const float blocksize /* number of pixels in a block */             = block_d * block_d; // since our blocks are square

        int64_t pblocksize_right = // number of pixels in each block in the rightmost column of incomplete blocks.
            // width of the image - (number of complete blocks * block dimension) will give the residual pixels along the horizontal axis
            // multiply that by block domension again, and we'll get the number of pixels in the incomplete block
            (image->_infoheader.biWidth -
             (image->_infoheader.biWidth / block_d) /* deliberate integer division to get only the count of complete blocks */ * block_d) *
            block_d;
        assert(pblocksize_right < blocksize);

        // the block size to represent the number of pixels held by the last row blocks
        int64_t pblocksize_bottom = (image->_infoheader.biHeight - (image->_infoheader.biHeight / block_d) * block_d) * block_d;
        assert(pblocksize_bottom < blocksize);

        const int64_t nblocks_w = ceill(image->_infoheader.biWidth / (float) block_d);
        const int64_t nblocks_h = ceill(image->_infoheader.biHeight / (float) block_d);

        // we have to compute the average R, G & B values for all pixels inside each pixel blocks and use the average to represent
        // that block as a wchar_t. one wchar_t in our buffer will have to represent (block_w x block_h) number of RGBQUADs
        const int64_t nwchars   = nblocks_h * (nblocks_w + 2) + 1; // saving two wide chars for CRLF!, the +1 is for the NULL terminator

        wchar_t* const buffer   = malloc(nwchars * sizeof(wchar_t));
        if (!buffer) {
            fwprintf_s(stderr, L"Error in %s @ line %d: malloc failed!\n", __FUNCTIONW__, __LINE__);
            return NULL;
        }

        // NOLINTBEGIN(readability-isolate-declaration)
        float blockavg_blue = 0.0F, blockavg_green = 0.0F,
              blockavg_red = 0.0F; // per block averages of the rgbBlue, rgbGreen and rgbRed values
        int64_t    caret = 0, offset = 0, col = 0, row = 0;
        const bool block_rows_end_with_incomplete_blocks =
            image->_infoheader.biWidth % CONSOLE_WIDTH; // true if the image width is not divisible by 140 without remainders
        const bool block_columns_end_with_incomplete_blocks =
            image->_infoheader.biHeight % block_d; // true if the image height is not divisible by block_d without remainders
        // NOLINTEND(readability-isolate-declaration)

        uint64_t full = 0, incomplete = 0, count = 0; // NOLINT(readability-isolate-declaration)

        // row = image->_infoheader.biHeight will get us to the last pixel of the first (last in the buffer) scanline with (r * image->_infoheader.biWidth)
        // hence, row = image->_infoheader.biHeight - 1 so we can traverse pixels in the first scanline with (r * image->_infoheader.biWidth) + c
        for (row  = image->_infoheader.biHeight - 1; row >= (block_d - 1);
             row -= block_d) {                                                           // start the traversal at the bottom most scan line
                                                                                         // wprintf_s(L"row = %lld\n", row);
            for (col = 0; col <= image->_infoheader.biWidth - block_d; col += block_d) { // traverse left to right in scan lines
                // wprintf_s(L"row = %lld, col = %lld\n", row, col);

                for (int64_t r = row; r > row - block_d; --r) { // deal with blocks
                    for (int64_t c = col; c < col + block_d; ++c) {
                        offset          = (r * image->_infoheader.biWidth) + c;
                        blockavg_blue  += image->_pixels[offset].rgbBlue;
                        blockavg_green += image->_pixels[offset].rgbGreen;
                        blockavg_red   += image->_pixels[offset].rgbRed;
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

                for (int64_t r = row; r > row - block_d; --r) {
                    // shift the column delimiter backward by one block, to the end of the last complete block
                    for (int64_t c = col; c < image->_infoheader.biWidth; ++c) { // start from the end of the last complete block
                        offset          = (r * image->_infoheader.biWidth) + c;
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

            for (col = 0; col < image->_infoheader.biWidth; col += block_d) { // col must be 0 at the start of this loop

                for (int64_t r = row; r >= 0; --r) {                // r delimits the start row of the block being defined
                    for (int64_t c = col; c < col + block_d; ++c) { // c delimits the start column of the block being defined
                        offset          = (r * image->_infoheader.biWidth) + c;
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

        assert(caret == nwchars);

        return buffer;
    }

    // an image width predicated dispatcher for to_raw_string and to_downscaled_string
    static inline wchar_t* __cdecl to_string(const bitmap_t* const image) {
        if (image->_infoheader.biWidth <= CONSOLE_WIDTH) return to_raw_string(image);
        return to_downscaled_string(image);
    }
} // namespace _to_text_impl

// NOLINTEND(readability-redundant-inline-specifier,modernize-avoid-c-arrays)
