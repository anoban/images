#pragma once
#ifndef __GIF87A_H_
    #define __GIF87A_H_

// In 1987 CompuServe published a GIF (Graphics Interchange Format) called GIF87a and was unanimously adopted my most image processing applications.
//CompuServe later published a revision to this standard in 1989 called GIF89a. But most GIF images only uses specs defined by GIF87a.
// Two main features of GIF images that set them apart from all other image formats are:
//      1) Up to 256 colours using 1 to 8 bit per pixel
//      2) Storing multiple static image files in a single .gif file

// In contrast to other image encodings GIF uses Little Endian byte ordering for multi byte values, so no need for additional bit twiddling operations
// on Intel LE systems.

#endif