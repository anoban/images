#include <bitmap>
#include <canvas>

int wmain(_In_ int argc, _In_ const wchar_t* const wargv[]) {
    //
    if (argc != 2) return EXIT_FAILURE;

    canvas image { bitmap { wargv[1] } };
    image.to_negative().to_file(LR"(bw.bmp)");
    return EXIT_SUCCESS;
}
