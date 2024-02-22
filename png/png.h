#pragma once
#ifndef __PNG_H_
    #define __PNG_H_
    #include <stdint.h>

/*
    PNG (Portable Network Graphics) is the newest image format compared to JPEG, BMP and GIF
    PNG uses loseless compression and supports:
        1) Usage of up to 48 bits to represent a pixel
        2) 1, 2, 4, 8, 16 bit sample precisions
        3) Alpha channel for full colour transparency
        4) Complicated colour matching ??    
        
    PNG stores multibyte integers in MSB first order (Big Endian)
    Bit strings are read from the least to the most significant bit ??
    When a bit string steps over a byte boundary, bits in the second
*/

typedef struct chunk {
        uint32_t length;  // size of the data segment of a given chunk, in bytes (Big Endian format)
                          // doesn't include any other parts of the chunk
        char     type[4]; // PNG chunk type, usually encoded as 4 consecutive ASCII characters
        void*    data;
        uint32_t crc32;
} chunk_t;

#endif