#include <bmp.h>
#include <heapapi.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    const HANDLE64 hProcHeap = GetProcessHeap();
    bmp_t          image;
    for (int64_t i = 1; i < argc; ++i) {
        image = BmpRead(argv[i]);
        // do something
        HeapFree(hProcHeap, 0, image.pixels);
    }
    return EXIT_SUCCESS;
}
