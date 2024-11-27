#include <canvas>
#include <format>
#include <fractal>

int wmain(_In_opt_ int argc, _In_opt_count_c_(argc) const wchar_t* const argv[]) {
    fractal image { 100, 100 };
    image.waves();
    image.to_file(LR"(.\waves.bmp)");
    image.mandelbrot(colourmaps::COPPER);
    image.to_file(LR"(.\mandelbrot.bmp)");

    return EXIT_SUCCESS;
}
