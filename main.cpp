#include <canvas>

int main() {
    canvas image { LR"(./corvette.bmp)" };
    image.hflip().to_file(LR"(./hflipped.bmp)");

    return EXIT_SUCCESS;
}
