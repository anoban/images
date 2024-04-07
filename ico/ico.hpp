#if !(defined __cplusplus) || (__cplusplus < 202002L)
    #error "The ICO subproject requires a C++20 or later standards compliant compiler!"
#endif

#pragma once
#ifndef __ICO_HPP__
    #define __ICO_HPP__

    #include <array>
    #include <cassert>
    #include <iostream>
    #include <optional>
    #include <vector>

namespace ico {

    constexpr std::size_t MAX_ICONDIRENTRIES { 10 };

    // an ICO file can be imagined as a metainfo struct, called ICONDIR, for ICON DIRectory followed by a bitmap or an array of bitmaps
    // (.ico files can contain one or more images)
    // these bitmap images are stored in contiguously following the ICONDIR structure.
    // each bitmap is defined by an ICONDIRENTRY struct in the ICONDIR struct
    // the bitmap data can be in the format of a Windows BMP file without the BITMAPFILEHEADER struct or a PNG image in its entirety i.e uncompressed.

    // in summary the binary representation of an .ico file looks like
    // ICONDIR, BITMAP data

    // emulating Win32 definitions, names and types for syntactic consistency
    using BYTE  = unsigned char;  // 8 bits
    using WORD  = unsigned short; // 16 bits
    using DWORD = unsigned long;  // 32 bits

    enum class IMAGETYPE : WORD { ICO = 1 /* icon */, CUR = 2 /* cursor */ };

    // look up Raymond Chen's article https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083 for reference.

    struct ICONDIRENTRY {
            /*
            Win32 uses the following definition ::
            typedef struct GRPICONDIRENTRY
            {
                BYTE  bWidth;
                BYTE  bHeight;
                BYTE  bColorCount;
                BYTE  bReserved;
                WORD  wPlanes;
                WORD  wBitCount;
                DWORD dwBytesInRes;
                WORD  dwImageOffset;
            }
            */

            BYTE  bWidth {};      // width of the associated bitmap in pixels (must be in the range of 0 to 256)
            BYTE  bHeight {};     // height of the associated bitmap in pixels (must be in the range of 0 to 256)
            BYTE  bColorCount {}; // number of colours in the colur palette, must be 0 if the bitmap doesn't use a colour palette.
            BYTE  bReserved {};   // reserved byte, must always be 0.
            WORD  wPlanes {};     // specifie the colour planes (should be 0 or 1) - for ICO bitmaps
            // wPlanes specifies the horizontal coordinate of the hotspot as offset from the left, in pixels - for CUR bitmaps
            WORD  wBitCount {}; // specifies bits per pixel - for ICO bitmaps
            // wBitCount specifies the vertical coordinate of the hotspot as offset from the top, in pixels - for CUR bitmaps
            // Windows cursors have a hotspot location, that decides one exact point that is affected by mouse events!
            // https://learn.microsoft.com/en-us/windows/win32/menurc/about-cursors
            DWORD dwBytesInRes {};  // size of the associated bitmap in bytes
            DWORD dwImageOffset {}; // offset of the associated bitmap data, from the beginning of the .ico or .cur file
    };

    struct ICONDIR {
            /*
            Win32 uses the following definition ::
            typedef struct GRPICONDIR
            {
                WORD idReserved;
                WORD idType;
                WORD idCount;
                GRPICONDIRENTRY idEntries[];
            } GRPICONDIR
            */

            // id prefix for IconDirectory, classic Win32 stuff
            WORD      idReserved {}; // reserved field, must always be 0
            IMAGETYPE idType {};     // specifies the type of the resources contained, values other than 1 and 2 are invalid
            WORD      idCount {};    // number of resources (images) stored in the given .ico file
            std::array<ICONDIRENTRY, MAX_ICONDIRENTRIES> idEntries {}; // not going to the heap for this.
            // if a .ico or .cur file is suspected to store more than 10 bitmaps, manually adjust MAXICNDRENTRYS to a higher value!
    };

    class ico {
            // represents an .ico file object

        public:
            ico() = delete; // no default constructor
            ico(_In_ const wchar_t* const filename) noexcept;

        private:
            ICONDIR              icDirectory;
            std::vector<uint8_t> buffer;
    }; // class ico

    namespace io {

        std::optional<std::vector<uint8_t>> Open(_In_ const wchar_t* const filename) noexcept;
        bool                                Serialize(_In_ const wchar_t* const filename, _In_ const std::vector<uint8_t>& buffer) noexcept;

    } // namespace io

} // namespace ico

#endif // !__ICO_HPP__
