#pragma once
#ifndef __BMP_H_
    #define __BMP_H_
    #include <stdbool.h>
    #include <stdint.h>
    // these data structures are implemented for learning purpose, but the functions are designed to operate with WinGdi structs
    // not these implementations
    #ifndef __USE_HANDROLLED_BMP_STRUCTS__ //  DON'T UNLESS ABSOLUTELY NECESSARY
        #define _AMD64_                    // architecture
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
        #include <windef.h>
        #include <wingdi.h>
    #endif // !__USE_HANDROLLED_BMP_STRUCTS__

    #pragma comment(lib, "Gdi32.lib")

/*
   BMP format supports 1, 4, 8, 16, 24 and 32 bits per pixel.
   Even though Windows BMP format supports simple run length compression for 4 or 8 bits per pixel, it's rarely useful since
   it can only be used with large pixel blocks of identical colours.
   Multibyte integers in Windows BMP are stored LSB first (LE byte  order)
*/

// Since BMP format originted in Microsoft, Windows SDK comes pre-packed with almost all necessary data structures and routines
// needed to read in and process

// every Windows BMP begins with a BITMAPFILEHEADER struct
// this helps in recognizing the file format as .bmp
// the first two bytes will be 'B', 'M'

// all these data structures are provided in wingdi.h, so let's just use them instead of reinventing the wheel.
    #ifdef __USE_HANDROLLED_BMP_STRUCTS__ // JUST DON'T

        #pragma region _HANDROLLED_STRUCTS_
    // #pragma pack directive is a risky business, will likely impede gratuitous runtime performance penalties
        #pragma pack(push, 1)
typedef struct {
        uint8_t  SOI[2];   // 'B', 'M'
        uint32_t FSIZE;
        uint32_t RESERVED; // this is actually two consecutive 16 bit elements, but who cares :)
        uint32_t PIXELDATASTART;
} BITMAPFILEHEADER;
        #pragma pack(pop)

    // image header comes in two variants!
    // one representing OS/2 BMP format (BITMAPCOREHEADER) and another representing the most common Windows BMP format.
    // (BITMAPINFOHEADER)
    // however there are no markers to identify the type of the image header present in the bmp image.
    // the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct.
    // sizeof BITMAPCOREHEADER is 12 bytes
    // sizeof BITMAPINFOHEADER is >= 40 bytes.
    // from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes!

        #pragma pack(push, 1)
typedef struct {
        uint32_t        HEADERSIZE; // >= 40 bytes.
        uint32_t        WIDTH;      // width of the bitmap image in pixels
        int32_t         HEIGHT;     // height of the bitmap image in pixels
        // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
        // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
        uint16_t        NPLANES;       // must be 1
        uint16_t        NBITSPERPIXEL; // 1, 4, 8, 16, 24 or 32
        COMPRESSIONKIND CMPTYPE;       // compression kind
        uint32_t        IMAGESIZE;     // 0 if not compressed.
        uint32_t        RESPPMX;       // resolution in pixels per meter along x axis.
        uint32_t        RESPPMY;       // resolution in pixels per meter along y axis.
        uint32_t        NCMAPENTRIES;  // number of entries in the colourmap that are used.
        uint32_t        NIMPCOLORS;    // number of important colors.
} BITMAPINFOHEADER;
        #pragma pack(pop)

    // a BMP with BITMAPCOREHEADER cannot be compressed and is very rarely used in modern BMPs.

        #pragma pack(push, 1)
typedef struct {
        uint32_t HEADERSIZE; // 12 bytes
        uint16_t WIDTH;
        uint16_t HEIGHT;
        uint16_t NPLANES;       // must be 1
        uint16_t NBITSPERPIXEL; // could be 1, 4, 8 or 24
} BITMAPCOREHEADER;

// struct used to represent RGB pixels in BMP images (more widely uised compared to RGBTRIPLE)
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
        uint8_t RESERVED; // must be 0, but seems to be 0xFF in most BMPs.
} RGBQUAD;

// BMP files in OS/2 use this variant.
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
} RGBTRIPLE;
        #pragma endregion _HANDROLLED_STRUCTS_
    #endif // !__USE_HANDROLLED_BMP_STRUCTS__

// order of pixels in the BMP buffer.
typedef enum { TOPDOWN, BOTTOMUP } BMPPIXDATAORDERING;

// mechanism to be used in converting RGB pixels to black and white.
typedef enum { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY } TOBWKIND;

// combinations of RGBs to remove from pixels.
typedef enum { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE } RGBCOMB;

// types of compressions used in BMP files.
typedef enum { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN } COMPRESSIONKIND;

// a struct representing a BMP image
typedef struct bmp {
        BITMAPFILEHEADER fileheader;
        BITMAPINFOHEADER infoheader;
        RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e (buffer + 54)
        uint8_t*         buffer; // this will point to the original file buffer, this is the one that needs deallocation!
} bmp_t;

// prototypes
bmp_t BmpRead(_In_ const wchar_t* const restrict filepath);
bool  BmpWrite(_In_ const wchar_t* const restrict filepath, _In_ bmp_t* const restrict image, _In_ const bool cleanup);
void  BmpInfo(_In_ const bmp_t* const image);
bmp_t ToBWhite(_In_ bmp_t* const image, _In_ const TOBWKIND conversionkind, _In_ const bool inplace);
bmp_t ToNegative(_In_ bmp_t* const image, _In_ const bool inplace);
bmp_t RemoveColour(_In_ bmp_t* const image, _In_ const RGBCOMB colourcombination, _In_ const bool inplace);
bmp_t GenGradient(_In_ const size_t npixels_h, _In_ const size_t npixels_w);

#endif // !__BMP_H_
