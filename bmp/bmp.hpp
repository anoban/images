#pragma once

#ifndef __BMP_H_
    #define __BMP_H_

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

namespace bmp {

    constexpr std::array<uint8_t, 2> soi {
        {'B', 'M'}
    };

    #pragma pack(push, 1)
    struct BITMAPFILEHEADER {
            std::array<uint8_t, 2> SOI {};      // 'B', 'M'
            uint32_t               FSIZE {};
            uint32_t               RESERVED {}; // this is actually two consecutive 16 bit elements, but who cares :)
            uint32_t               PIXELDATASTART {};
    };
    #pragma pack(pop)

    enum class COMPRESSIONKIND : uint32_t { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN }; // uint32_t

    #pragma pack(push, 2)
    struct BITMAPINFOHEADER {
            uint32_t        HEADERSIZE {};                                         // >= 40 bytes.
            uint32_t        WIDTH {};
            int32_t         HEIGHT {}; // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
            // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
            uint16_t        NPLANES {};       // must be 1
            uint16_t        NBITSPERPIXEL {}; // 1, 4, 8, 16, 24 or 32
            COMPRESSIONKIND CMPTYPE {};
            uint32_t        IMAGESIZE {};     // 0 if not compressed.
            uint32_t        RESPPMX {};       // resolution in pixels per meter along x axis.
            uint32_t        RESPPMY {};       // resolution in pixels per meter along y axis.
            uint32_t        NCMAPENTRIES {};  // number of entries in the colourmap that are used.
            uint32_t        NIMPCOLORS {};    // number of important colors.
    };
    #pragma pack(pop)

    struct BITMAPCOREHEADER {
            uint32_t HEADERSIZE {};           // 12 bytes
            uint16_t WIDTH {};
            uint16_t HEIGHT {};
            uint16_t NPLANES {};              // must be 1
            uint16_t NBITSPERPIXEL {};        // 1, 4, 8 or 24
    };

    #pragma pack(push, 1)
    struct RGBQUAD {
            uint8_t BLUE {};
            uint8_t GREEN {};
            uint8_t RED {};
            uint8_t RESERVED {}; // must be 0, but seems to be 0xFF in most BMPs, yikes!
    };
    #pragma pack(pop)

    // BMP files in OS/2 use the third variant
    struct RGBTRIPLE {
            uint8_t BLUE {};
            uint8_t GREEN {};
            uint8_t RED {};
    };

    enum class BMPPIXDATAORDERING { TOPDOWN, BOTTOMUP };

    enum class TOBWKIND : uint32_t { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY };

    enum class RGBCOMB : uint32_t { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE };

    class bmp {
        private:
            size_t                                              size {};
            size_t                                              npixels {};
            BITMAPFILEHEADER                                    fh {};
            BITMAPINFOHEADER                                    infh {};
            std::vector<RGBQUAD>                                pixels {};

            [[msvc::forceinline, nodiscard]] BITMAPFILEHEADER   parse_fileheader(_In_ const std::vector<uint8_t>& imstream);

            [[msvc::forceinline, nodiscard]] COMPRESSIONKIND    get_compressionkind(_In_ const uint32_t cmpkind) noexcept;

            [[msvc::forceinline, nodiscard]] BITMAPINFOHEADER   parse_infoheader(_In_ const std::vector<uint8_t>& imstream);

            [[msvc::forceinline, nodiscard]] BMPPIXDATAORDERING get_pixelorder(_In_ const BITMAPINFOHEADER& header) noexcept;

        public:
            bmp(void) = default;
            [[msvc::forceinline, nodiscard]] bmp(_In_ const std::wstring& path);

            [[msvc::forceinline, nodiscard]] constexpr bmp(
                _In_ const BITMAPFILEHEADER& headf, _In_ const BITMAPINFOHEADER& headinf, _In_ const std::vector<RGBQUAD>& pbuff
            ) noexcept;

            [[msvc::forceinline]] void                          serialize(_In_ const std::wstring& path);

            [[msvc::forceinline, msvc::flatten]] void           info(void) noexcept;

            [[msvc::forceinline, nodiscard]] std::optional<bmp> tobwhite(
                _In_ const TOBWKIND cnvrsnkind, _In_opt_ const bool inplace = false
            ) noexcept;

            [[msvc::forceinline, nodiscard]] std::optional<bmp> tonegative(_In_opt_ const bool inplace = false) noexcept;

            [[msvc::forceinline, nodiscard]] std::optional<bmp> remove_clr(
                _In_ const RGBCOMB kind, _In_opt_ const bool inplace = false
            ) noexcept;

            [[msvc::noinline, nodiscard]] static bmp gengradient(
                _In_opt_ const size_t heightpx = 1080, _In_opt_ const size_t widthpx = 1080
            ) noexcept;

    }; // class bmp

    [[msvc::forceinline, nodiscard]] std::vector<uint8_t>      open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes);
    // expects argv to be passed as fnames (without any preprocessing) i.e do not remove the first element in argv.

    [[msvc::forceinline, nodiscard]] std::vector<std::wstring> remove_ext(
        _In_count_(size) wchar_t* const fnames[], _In_ const size_t length, _In_ const wchar_t* const extension
    ) noexcept;

} // namespace bmp

#endif // !__BMP_H_
