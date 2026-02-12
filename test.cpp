#include <bitmap>
#include <canvas>

int main(int argc, const char* const argv[]) {
    //
    if (argc != 2) return EXIT_FAILURE;

    canvas image { bitmap { wargv[1] } };
    image.to_negative().to_file(LR"(bw.bmp)");
    return EXIT_SUCCESS;
}
