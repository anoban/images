#include <png.h>

bool __stdcall IsValidPngChunk(_In_ const chunk_t* const restrict chunk) {
    return isascii(chunk->pcType[0]) && isascii(chunk->pcType[1]) && isascii(chunk->pcType[2]) && isascii(chunk->pcType[3]);
}