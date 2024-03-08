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
        // when file paths are passed with a .\ this period will be captured as the first period, which we do not want to happen
        // assumes all files are passed in the format .\<file path>
        periodpos               = wcschr(fname + 2, L'.');

        const bmp_t bwlum_image = ToBWhite(&image, LUMINOSITY, false);
        swprintf_s(periodpos, 20, L"_lum.bmp");
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

        const bmp_t bwbin_image = ToBWhite(&image, BINARY, false);
        swprintf_s(periodpos, 20, L"_bin.bmp");
        BmpWrite(fname, &bwbin_image);
        HeapFree(hProcHeap, 0, bwbin_image.buffer);

        const bmp_t bg_image = RemoveColour(&image, RED, false);
        swprintf_s(periodpos, 20, L"_bg.bmp");
        BmpWrite(fname, &bg_image);
        HeapFree(hProcHeap, 0, bg_image.buffer);

        const bmp_t br_image = RemoveColour(&image, GREEN, false);
        swprintf_s(periodpos, 20, L"_br.bmp");
        BmpWrite(fname, &br_image);
        HeapFree(hProcHeap, 0, br_image.buffer);

        const bmp_t rg_image = RemoveColour(&image, BLUE, false);
        swprintf_s(periodpos, 20, L"_rg.bmp");
        BmpWrite(fname, &rg_image);
        HeapFree(hProcHeap, 0, rg_image.buffer);

        const bmp_t r_image = RemoveColour(&image, GREENBLUE, false);
        swprintf_s(periodpos, 20, L"_r.bmp");
        BmpWrite(fname, &r_image);
        HeapFree(hProcHeap, 0, r_image.buffer);

        const bmp_t g_image = RemoveColour(&image, REDBLUE, false);
        swprintf_s(periodpos, 20, L"_g.bmp");
        BmpWrite(fname, &g_image);
        HeapFree(hProcHeap, 0, g_image.buffer);

        const bmp_t b_image = RemoveColour(&image, REDGREEN, false);
        swprintf_s(periodpos, 20, L"_b.bmp");
        BmpWrite(fname, &b_image);
        HeapFree(hProcHeap, 0, b_image.buffer);

        ToNegative(&image, true);
        swprintf_s(periodpos, 20, L"_neg.bmp");
        BmpWrite(fname, &image);
        HeapFree(hProcHeap, 0, image.buffer);
    }

    return EXIT_SUCCESS;
}
