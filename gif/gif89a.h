#pragma once
#ifndef __GIF89A_H_
    #define __GIF89A_H_
    #include <stdint.h>

// In 1987 CompuServe published a GIF (Graphics Interchange Format) called GIF87a and was unanimously adopted my most image processing applications.
// CompuServe later published a revision to this standard in 1989 called GIF89a. But most GIF images only uses specs defined by GIF87a.
// Two main features of GIF images that set them apart from all other image formats are:
//      1) Up to 256 colours using 1 to 8 bit per pixel
//      2) Storing multiple static image files in a single .gif file

// In contrast to other image encodings GIF uses Little Endian byte ordering for multi byte values,
// so no need for additional bit twiddling operations on Intel LE systems.

// GIF format generally uses the LZW compression algorithm

/*
    Each GIF file must contain a GIF header at the start of the binary byte stream.
    This header allows applications to distinguish a .GIF file and identify its version.
    Applications that do not comply with GIF89a usually opt for a GIF87a standard GIF header
*/

typedef struct gifhead {
        char signature[3]; // 'G', 'I', 'F'
        char version[3];   // can be '8', '7', 'a' or '8', '9', 'a'
} gifhead_t;

/*
    Next part of a typical GIF file is the Logical Screen Descriptor (LSD)
    This defines the screen area where the images stored in GIF files are displayed.
    Individual Image Descriptors (IID) specify where are these images to be placed in the logical screen.
*/

typedef struct lsd {
        uint16_t width;
        uint16_t height;
        uint8_t  bitfields;
        uint8_t  gctabsize   : 3; // 2^(N + 1) gives the number of entries in the global colour table
        uint8_t ctabsortflag : 1; // a flag registering whether the colours in the global colour table are sorted in the order of importance
        uint8_t bitsperpixel : 3; // bit depth - 1
        uint8_t gctabflag    : 1; // a flag registering whether there's a global colour table present
        uint8_t backgroundcolor;  // offset to the global colour table
        uint8_t pixelaspectratio; // if not 0, pixel width and pixel height are not equal
} lsd_t;

#endif // !__GIF89A_H_