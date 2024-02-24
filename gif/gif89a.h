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

// GIF format generally uses the LZW compression.

/*
    Each GIF file must contain a GIF header at the start of the binary byte stream.
    This header allows applications to distinguish a .GIF file and identify its version.
    Applications that do not comply with GIF89a usually opt for a GIF87a standard GIF header
*/

// succumbing to using Windows's naming conventions as I don't want functions look out of place with the Win32 routines they
// interface with which religiously use Pascal case with Hungarian notation.
// GIF file header
typedef struct gifhead {
        char ghSignature[3]; // the ASCII string "GIF"
        char ghVersion[3];   // can be "87a" or "89a"
} gifhead_t;

/*
    Next part of a typical GIF file is the Logical Screen Descriptor (LSD)
    This defines the screen area where the images stored in GIF files are displayed.
    Individual Image Descriptors (IID) specify where are these images to be placed in the logical screen.
*/

// GIF Logical Screen Descriptor
typedef struct lsd {
        uint16_t glWidth;
        uint16_t glHeight;
        uint8_t  glBitfields;
        uint8_t  glGctabsize    : 3; // 2^(N + 1) gives the number of entries in the global colour table
        uint8_t  glCtabsortflag : 1;
        // a flag registering whether the colours in the global colour table are sorted in the order of importance
        uint8_t  glBitsperpixel : 3; // bit depth - 1
        uint8_t  glGctabflag    : 1; // a flag registering whether there's a global colour table present
        uint8_t  glBackgroundcolor;  // offset to the global colour table
        uint8_t  glPixelaspectratio; // if not 0, pixel width and pixel height are not equal
} lsd_t;

#endif // !__GIF89A_H_