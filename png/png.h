#pragma once
#ifndef __PNG_H_
    #define __PNG_H_

    #include <stdint.h>

typedef struct chunk {
        uint32_t length;  // size of the data segment of a given chunk, in bytes (Big Endian formmat)
                          // doesn't include any other parts of the chunk
        char     type[4]; // PNG chunk type, usually encoded as 4 consecutive ASCII characters
        void*    data;
        uint32_t crc32;
} chunk_t;

#endif