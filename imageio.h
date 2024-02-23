// a generic, image format agnostic binary file IO routines
// exposes only the prototypes, implementations in imageio.c

#pragma once
#ifndef __IMAGE_IO_H_
    #define __IMAGE_IO_H_
    #include <stdbool.h>
    #include <stdint.h>

uint8_t* Open(_In_ const wchar_t* const restrict file_name, _Inout_ size_t* const restrict size);

bool     Serialize(
        _In_ const wchar_t* const restrict filename,
        _In_ const uint8_t* const restrict buffer,
        _In_ const size_t size,
        _In_ const bool   free_after_use /* specifies whether to free the buffer after serialization */
    );

#endif // !__IMAGE_IO_H_