#include <canvas>
#include <fractals>

int wmain() {
    // bitmap image { LR"(.\butterflies.bmp)" };
    // std::wcout << image;

    // const canvas board { std::move(image) };

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

    // ::create_mandelbrot_set();
    fractals::tricorn(2000, 2000).to_file(LR"(.\tricorn.bmp)");

    return EXIT_SUCCESS;
}
