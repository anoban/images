// a generic, image format agnostic binary file IO routines
// exposes only the prototypes, implementations in imageio.c

#pragma once
#ifndef __IMAGE_IO_H_
    #define __IMAGE_IO_H_
    #include <stdint.h>

uint8_t* open(_In_ const wchar_t* const restrict file_name, _Inout_ size_t* const restrict size);

#endif // !__IMAGE_IO_H_