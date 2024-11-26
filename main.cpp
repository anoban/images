#include <canvas>
#include <format>
#include <fractal>

int wmain(_In_opt_ int argc, _In_opt_count_c_(argc) const wchar_t* const argv[]) {
    fractal image { 2000, 1000 };
    image.waves();
    image.to_file(LR"(.\waves.bmp)");

    // board.to_blacknwhite<rgb::BW_TRANSFORMATION::AVERAGE>().to_file(LR"(average.bmp)");
    // board.to_blacknwhite<rgb::BW_TRANSFORMATION::WEIGHTED_AVERAGE>().to_file(LR"(weighted_average.bmp)");
    // board.to_blacknwhite<rgb::BW_TRANSFORMATION::LUMINOSITY>().to_file(LR"(luminosity.bmp)");
    // board.to_blacknwhite<rgb::BW_TRANSFORMATION::BINARY>().to_file(LR"(binary.bmp)");

    // board.remove_colour<rgb::RGB_TAG::RED>().to_file(LR"(bluegreen.bmp)");
    // board.remove_colour<rgb::RGB_TAG::GREEN>().to_file(LR"(redblue.bmp)");
    // board.remove_colour<rgb::RGB_TAG::BLUE>().to_file(LR"(redgreen.bmp)");
    // board.remove_colour<rgb::RGB_TAG::GREENBLUE>().to_file(LR"(red.bmp)");
    // board.remove_colour<rgb::RGB_TAG::REDBLUE>().to_file(LR"(green.bmp)");
    // board.remove_colour<rgb::RGB_TAG::REDGREEN>().to_file(LR"(blue.bmp)");

    // canvas fractal(5000, 5000);
    // fractal.mandelbrot(colourmaps::COPPER);
    // fractal.to_file(LR"(.\mandelbrot.bmp)");

    return EXIT_SUCCESS;
}
