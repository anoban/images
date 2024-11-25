#include <canvas>

int wmain() {
    bitmap image { LR"(.\sweeney.bmp)" };

    std::wcout << image;

    canvas board { std::move(image) };
    board.vflip();
    board.to_file(LR"(vflipped.bmp)");

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
