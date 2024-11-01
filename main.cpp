#include <bitmap>

int wmain() {
    const bitmap image { LR"(black.bmp)" };
    image.make_negative().to_file(LR"(negative.bmp)");

    image.make_blacknwhite(BW_TRANSFORMATION::AVERAGE).to_file(LR"(average.bmp)");
    image.make_blacknwhite(BW_TRANSFORMATION::WEIGHTED_AVERAGE).to_file(LR"(average.bmp)");
    image.make_blacknwhite(BW_TRANSFORMATION::LUMINOSITY).to_file(LR"(luminosity.bmp)");
    image.make_blacknwhite(BW_TRANSFORMATION::BINARY).to_file(LR"(binary.bmp)");

    image.make_decoloured<RGB_TAG::BLUE>().to_file(LR"(redgreen.bmp)");
    image.make_decoloured<RGB_TAG::GREEN>().to_file(LR"(redblue.bmp)");
    image.make_decoloured<RGB_TAG::RED>().to_file(LR"(bluegreen.bmp)");
    image.make_decoloured<RGB_TAG::REDBLUE>().to_file(LR"(green.bmp)");
    image.make_decoloured<RGB_TAG::REDGREEN>().to_file(LR"(blue.bmp)");
    image.make_decoloured<RGB_TAG::GREENBLUE>().to_file(LR"(red.bmp)");
    return EXIT_SUCCESS;
}
