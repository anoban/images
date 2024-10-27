#pragma once
#ifndef __PNG_H_
    #define __PNG_H_
    #include <ctype.h> // isupper()
    #include <stdbool.h>
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
    When a bit string steps over a byte boundary, bits in the second byte become the most significant ??
    
    Huffman codes within the compressed data are stored with their bits reversed.
    MSB of the Huffman codes will be the LSB of the data byte and vice versa.
    
    A PNG file is composed of a string of chunks.
    PNG chunks can be defined by three entities:
        1) The PNG standard - most important chunks any decoder must be capable of parsing
        2) Public - A list of chunk specs submitted by the public and accepted by the PNG standard
        3) Private - Chunks defined by applications e.g. Adobe PhotoShop (often intended for their own use)
        
    A PNG chunk consists of 4 parts:
        1) Length - Number of bytes in the data segment (BE byteorder, uint32_t)
        2) Type - Chunk name (4 characters)
        3) Data - a series of bytes (Length number of bytes in a sequence)
        4) CRC - A Cyclic Redundancy Check checksum value (BE byteorder, uint32_t)
        
    A decoder only needs to parse standard defined PNG chunks that follow the above structure. 
    It shall ignore chunks that are not in compliance with the above specified format, private chunks and newly incorporated public chunks.
    
    CHUNK NAMES
    
    PNG chunk names consist of 4 ASCII characters
    First, second and the fourth character in the name can be an uppercase or a lowercase ASCII character
    Third character must be a uppercase ASCII character
    Any chunks with non printing ASCII characters in its name must be deemed invalid.
    
*/

typedef struct chunk {
        uint32_t pcLength;  // size of the data segment of a given chunk, in bytes (Big Endian format)
                            // doesn't include any other parts of the chunk
        char     pcType[4]; // PNG chunk type, usually encoded as 4 consecutive ASCII characters
        uint32_t pcOffset;
        uint32_t pcChecksum;
} chunk_t;

// are all the characters in pcType, valid ASCII characters?.
bool __stdcall IsValidPngChunk(_In_ const chunk_t* const restrict chunk);

/*
    If the first alphabet of a chunk type is in capital, it is a CRITICAL chunk, that all decoders must be able to parse
    There are 4 critical chunks each PNG file is expected to contain:
        1) IHDR
        2) PLTE
        3) IDAT
        4) IEND
        
    Second character of public chunk types and chunk types approved by the PNG development group will be in uppercase.
    Private chunks defined by applications must have a lowercase second character, in order not to conflict with publicly defined chunks.
    
    COPY SAFE CHUNKS
    
    The last alphabet of the chunk name will be in lowercase, if the chunk is safe to copy.
    A decoder shall not attempt to copy non copy-safe chunks if it doesn't recognize them.
    
    e.g.
    
*/

#endif