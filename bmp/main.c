#include <bmp.h>
#include <heapapi.h>
#include <stdio.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    bmp_t    image;
    wchar_t  fname[MAX_PATH] = { 0 };
    wchar_t* periodpos       = NULL;

    for (int64_t i = 1; i < argc; ++i) {
        memset(fname, 0, MAX_PATH);
        image = BmpRead(argv[i]);

        swprintf_s(fname, MAX_PATH, L"%s", argv[i]);
        // when file paths are passed with a .\ this period will be captured as the first period, which we do not want to happen
        // assumes all files are passed in the format .\<file path>
        periodpos               = wcschr(fname + 2, L'.');

        const bmp_t bwlum_image = ToBWhite(&image, LUMINOSITY, false);
        swprintf_s(periodpos, 20, L"_lum.bmp");
        BmpWrite(fname, &bwlum_image, true);

        const bmp_t bwavg_image = ToBWhite(&image, AVERAGE, false);
        swprintf_s(periodpos, 20, L"_average.bmp");
        BmpWrite(fname, &bwavg_image, true);

        const bmp_t bwwavg_image = ToBWhite(&image, WEIGHTED_AVERAGE, false);
        swprintf_s(periodpos, 20, L"_waverage.bmp");
        BmpWrite(fname, &bwwavg_image, true);

        const bmp_t bwbin_image = ToBWhite(&image, BINARY, false);
        swprintf_s(periodpos, 20, L"_bin.bmp");
        BmpWrite(fname, &bwbin_image, true);

        const bmp_t bg_image = RemoveColour(&image, RED, false);
        swprintf_s(periodpos, 20, L"_bg.bmp");
        BmpWrite(fname, &bg_image, true);

        const bmp_t br_image = RemoveColour(&image, GREEN, false);
        swprintf_s(periodpos, 20, L"_br.bmp");
        BmpWrite(fname, &br_image, true);

        const bmp_t rg_image = RemoveColour(&image, BLUE, false);
        swprintf_s(periodpos, 20, L"_rg.bmp");
        BmpWrite(fname, &rg_image, true);

        const bmp_t r_image = RemoveColour(&image, GREENBLUE, false);
        swprintf_s(periodpos, 20, L"_r.bmp");
        BmpWrite(fname, &r_image, true);

        const bmp_t g_image = RemoveColour(&image, REDBLUE, false);
        swprintf_s(periodpos, 20, L"_g.bmp");
        BmpWrite(fname, &g_image, true);

        const bmp_t b_image = RemoveColour(&image, REDGREEN, false);
        swprintf_s(periodpos, 20, L"_b.bmp");
        BmpWrite(fname, &b_image, true);

        ToNegative(&image, true);
        swprintf_s(periodpos, 20, L"_neg.bmp");
        BmpWrite(fname, &image, true);
    }

    return EXIT_SUCCESS;
}
