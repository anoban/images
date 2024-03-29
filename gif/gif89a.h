#pragma once
#ifndef __GIF89A_H_
    #define __GIF89A_H_
    #include <stdint.h>

// In 1987 CompuServe published a GIF (Graphics Interchange Format) called GIF87a which was unanimously adopted my most image processing
// applications. CompuServe later published a revision of this standard in 1989 called GIF89a.
// Two main features of GIF images that set them apart from all other image formats are:
//      1) Up to 256 colours using 1 to 8 bit per pixel
//      2) Storing multiple static image files in a single .gif file

// In contrast to other image encodings GIF uses Little Endian byte ordering for multi byte values,
// so no need for additional bit twiddling operations on Intel LE systems.

// GIF format generally uses the LZW compression.

/*
    Each GIF file must contain a GIF header at the start of the binary byte stream.
    This header allows applications to distinguish a .GIF file and identify its version.
    Applications that do not comply with GIF89a usually opt for a GIF87a standard GIF header
*/

// Succumbing to using Win32's naming conventions as I don't want functions look out of place with the Win32 routines they
// interoperate with, which religiously use Pascal case with Hungarian notation.
// GIF file header
typedef struct GIFFILEHEADER {
        // gh prefix for gif head
        char ghSignature[3]; // the ASCII string "GIF"
        char ghVersion[3];   // can be "87a" or "89a"
} GIFFILEHEADER;

/*
    Next part of a typical GIF file is the Logical Screen Descriptor (LSD)
    This defines the screen area where the images stored in GIF files are displayed. This specifies the dimensions of the area and the background colour.
    Individual Image Descriptors (IID) specify where are these images to be placed in the logical screen.
*/

// GIF Logical Screen Descriptor
typedef struct GIFLSD {
        // gls for GIF Logical Screen
        uint16_t glsWidth;  // width of logical screen
        uint16_t glsHeight; // height of logical screen
        uint8_t  glsBitfields;
        uint8_t  glsGctabsize    : 3; // 2^(N + 1) gives the number of entries in the global colour table
        uint8_t  glsGctabsorted  : 1;
        // a flag registering whether the colours in the global colour table are sorted in the order of importance
        uint8_t  glsBitsperpixel : 3; // bit depth - 1, i.e if the bit depth is 8, this value will be 7
        uint8_t  glsGctab        : 1; // a flag registering whether there's a global colour table in the GIF image
        // if the struct is completely packed, members glsGctabsize, glsGctabsorted, glsBitsperpixel & glsGctab will collectively
        // occupy a byte.
        uint8_t  glsBackgroundcolor;  // offset into the global colour table (points to a color in the global colour table)
        uint8_t  glsPixelaspectratio; // if not 0, pixel width and pixel height are not equal
} GIFLSD;

// iIf the LSD registers the presence of a global colour table, then a global colour table immediately follows the LSD.
// Global colour table is an array of RGB structs. The number of RGB structs in global colour table is given by the glsGctabsize member of
// LSD struct. Unlike BMP, where the RGB members are ordered BGR, the struct members here must be in the RGB order. i.e WE CANNOT USE WINGDI STRUCTS

// WinGDI's RGBTRIPLE looks like this =>
// typedef struct tagRGBTRIPLE {
//      BYTE rgbtBlue;
//      BYTE rgbtGreen;
//      BYTE rgbtRed;
// } RGBTRIPLE, *PRGBTRIPLE, *NPRGBTRIPLE, *LPRGBTRIPLE;

typedef struct RGB {
        uint8_t rgbRed;
        uint8_t rgbGreen;
        uint8_t rgbBlue;
} RGBTRPL; // name RGBTRIPLE will lead to redefinition error with <wingdi.h>

// Following the global colour table, the variable part of GIF image begins.
// This section comprises of a series of images, that start with a hex code 0x2C. (no designated end codes here,
// next 0x2C notes the end of previous image and the start of next)

// GIF block codes, treat as 1 byte values
typedef enum { EXTENSION = 0x21, IMAGEBLOCK = 0x2C, TERMINATOR = 0x3B } GIFBLOCKS;

// Each image block (image) in GIF starts with a hex byte 0x2C and a image header follows this code.

typedef struct GIFIMAGEHEADER {
        uint16_t gihLposition;
        uint16_t gihTposition;
        uint16_t gihWidth;
        uint16_t gihHeight;
        uint8_t  gihBitdepth;
        uint8_t  gihLctabSize   : 3;
        uint8_t  gihReserved    : 2;
        uint8_t  gihLctabsorted : 1;
        uint8_t  gihInterlaced  : 1;
        uint8_t  gihLctab       : 1;
} GIFIMAGEHEADER;

#endif // !__GIF89A_H_
