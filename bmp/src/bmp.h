#pragma once
#ifndef __BMP_H_
    #define __BMP_H_

    #include <stdint.h>

static const uint8_t SOIMAGE[2] = { 'B', 'M' };

// every Windows BMP begins with a BITMAPFILEHEADER struct
// this helps in recognizing the file format as .bmp
// the first two bytes will be 'B', 'M'

typedef struct {
        uint8_t  SOI[2];   // 'B', 'M'
        uint32_t FSIZE;
        uint32_t RESERVED; // this is actually two consecutive 16 bit elements, but who cares :)
        uint32_t PIXELDATASTART;
} BITMAPFILEHEADER;

// types of compressions used in BMP files.
typedef enum { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN } COMPRESSIONKIND;

// image header comes in two variants!
// one representing OS/2 BMP format (BITMAPCOREHEADER) and another representing the most common Windows BMP format.
// (BITMAPINFOHEADER)
// however there are no markers to identify the type of the image header present in the bmp image.
// the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct.
// sizeof BITMAPCOREHEADER is 12 bytes
// sizeof BITMAPINFOHEADER is >= 40 bytes.
// from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes!

typedef struct {
        uint32_t        HEADERSIZE; // >= 40 bytes.
        uint32_t        WIDTH;
        int32_t         HEIGHT;     // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
        // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
        uint16_t        NPLANES;       // must be 1
        uint16_t        NBITSPERPIXEL; // 1, 4, 8, 16, 24 or 32
        COMPRESSIONKIND CMPTYPE;
        uint32_t        IMAGESIZE;     // 0 if not compressed.
        uint32_t        RESPPMX;       // resolution in pixels per meter along x axis.
        uint32_t        RESPPMY;       // resolution in pixels per meter along y axis.
        uint32_t        NCMAPENTRIES;  // number of entries in the colourmap that are used.
        uint32_t        NIMPCOLORS;    // number of important colors.
} BITMAPINFOHEADER;

// a BMP with BITMAPCOREHEADER cannot be compressed.
// and is very rarely used in modern BMPs.
typedef struct {
        uint32_t HEADERSIZE;    // 12 bytes
        uint16_t WIDTH;
        uint16_t HEIGHT;
        uint16_t NPLANES;       // must be 1
        uint16_t NBITSPERPIXEL; // could be 1, 4, 8 or 24
} BITMAPCOREHEADER;

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

typedef enum { TOPDOWN, BOTTOMUP } BMPPIXDATAORDERING;

typedef enum { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY } TOBWKIND;

typedef enum { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE } RGBCOMB;

typedef struct bmp {
        size_t           size;
        size_t           npixels;
        BITMAPFILEHEADER fhead;
        BITMAPINFOHEADER infhead;
        RGBQUAD*         pixels;
} bmp_t;

#endif // !__BMP_H_
