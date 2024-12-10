#include <canvas>

int main() {
    canvas image { LR"(./guitar.bmp)" };
    image.copy().vflip().to_file(LR"(./vflipped.bmp)");
    image.hflip_alt().to_file(LR"(./hflipped.bmp)");

    return EXIT_SUCCESS;
}
