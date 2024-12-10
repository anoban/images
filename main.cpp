#include <canvas>

int main() {
    canvas image { LR"(./scrambler.bmp)" };
    image.vflip().to_file(LR"(./upsidedown.bmp)");

    return EXIT_SUCCESS;
}
