#include <canvas>
#include <cmaps>

int main() {
    canvas board { 1080, 1920 };

    board.mandelbrot(colourmaps::JET);
    board.to_file(LR"(./mandelbrot_jet.bmp)");

    board.tricorn(colourmaps::JET);
    board.to_file(LR"(./tricorn_jet.bmp)");

    board.waves();
    board.to_file(LR"(./waves.bmp)");
    return EXIT_SUCCESS;
}
