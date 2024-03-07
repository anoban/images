#include <bmp.h>
#include <heapapi.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    const HANDLE64 hProcHeap = GetProcessHeap();
    bmp_t          image;
    for (int64_t i = 1; i < argc; ++i) {
        image               = BmpRead(argv[i]);
        const bmp_t bwimage = ToBWhite(&image, LUMINOSITY, false);
        BmpWrite(L",/bwimage.bmp", &bwimage);
        HeapFree(hProcHeap, 0, image.buffer);
        HeapFree(hProcHeap, 0, bwimage.buffer);
    }
    return EXIT_SUCCESS;
}
