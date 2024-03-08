#include <bmp.h>
#include <heapapi.h>
#include <stdio.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    const HANDLE64 hProcHeap = GetProcessHeap();
    bmp_t          image;
    wchar_t        fname[MAX_PATH] = { 0 };
    wchar_t*       periodpos       = NULL;

    for (int64_t i = 1; i < argc; ++i) {
        memset(fname, 0, MAX_PATH);
        image = BmpRead(argv[i]);

        swprintf_s(fname, MAX_PATH, L"%s", argv[i]);
        periodpos               = wcschr(fname, L'.');

        const bmp_t bwlum_image = ToBWhite(&image, LUMINOSITY, false);
        swprintf_s(periodpos, 20, L"_bw.bmp");
        BmpWrite(fname, &bwlum_image);
        HeapFree(hProcHeap, 0, bwlum_image.buffer);

        const bmp_t bwavg_image = ToBWhite(&image, AVERAGE, false);
        swprintf_s(periodpos, 20, L"_average.bmp");
        BmpWrite(fname, &bwavg_image);
        HeapFree(hProcHeap, 0, bwavg_image.buffer);

        const bmp_t bwwavg_image = ToBWhite(&image, WEIGHTED_AVERAGE, false);
        swprintf_s(periodpos, 20, L"_waverage.bmp");
        BmpWrite(fname, &bwwavg_image);
        HeapFree(hProcHeap, 0, bwwavg_image.buffer);

        ToBWhite(&image, BINARY, true /* modify the buffer inplace */);
        swprintf_s(periodpos, 20, L"_binary.bmp");
        BmpWrite(fname, &image);
        // since we no longer need the original buffer, we could reuse it for this.
        HeapFree(hProcHeap, 0, image.buffer);
    }

    return EXIT_SUCCESS;
}
