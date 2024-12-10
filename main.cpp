#include <canvas>

int main() {
    canvas board { 1080 * 8, 1920 * 8 };

    board.mandelbrot(colourmaps::VGA);
    board.to_file(LR"(./mandelbrot_vga.bmp)");

    return EXIT_SUCCESS;
}
