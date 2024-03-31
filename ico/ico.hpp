#pragma once
#ifndef __ICO_HPP__
    #define __ICO_HPP__

    #include <optional>
    #include <vector>

namespace ico {

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
    };

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
                WORD  nId;
            }
            */

            BYTE  bWidth {};      // width of the associated bitmap in pixels (must be in the range of 0 to 256)
            BYTE  bHeight {};     // height of the associated bitmap in pixels (must be in the range of 0 to 256)
            BYTE  bColorCount {}; // number of colours in the colur palette, must be 0 if the bitmap doesn't use a colour palette.
            BYTE  bReserved {};   // reserved byte, must always be 0.
            WORD  wPlanes {};     //
            WORD  wBitCount {};
            DWORD dwBytesInRes {}; // size of the associated bitmap in bytes
            DWORD dwImageOffset {};
    };

    class ico {
            // represents an .ico file object

        public:
            ico() = delete; // no default ctors
            inline ico(_In_ const wchar_t* const filename) noexcept;

        private:
    }; // class ico

    namespace io {
        std::optional<std::vector<uint8_t>> Open(_In_ const wchar_t* const filename) noexcept;
        bool                                Serialize(_In_ const wchar_t* const filename, _In_ const std::vector<uint8_t>& buffer) noexcept;
    } // namespace io

} // namespace ico

#endif // !__ICO_HPP__
