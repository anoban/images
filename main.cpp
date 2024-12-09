#include <canvas>
#include <chrono>
#include <cmaps>
#include <format>
#include <random>

int main() {
    std::mt19937_64 rand_engine { static_cast<unsigned long long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };
    std::uniform_int_distribution<long> runif { 2500, 3000 };
    canvas                              board { runif(rand_engine), runif(rand_engine) };

    // ::mandelbrot(board, colourmaps::JET);
    // board.to_file(LR"(./mandelbrot_jet.bmp)");
    // ::tricorn(board, colourmaps::JET);
    // board.to_file(LR"(./tricorn_jet.bmp)");

    canvas                              image { LR"(./geometry.bmp)" };
    image.copy().remove_colour<rgb::RGB_TAG::BLUE>().to_file(LR"(.\image_rg.bmp)");
    image.copy().remove_colour<rgb::RGB_TAG::RED>().to_file(LR"(.\image_bg.bmp)");
    image.copy().remove_colour<rgb::RGB_TAG::GREEN>().to_file(LR"(.\image_rb.bmp)");
    image.copy().remove_colour<rgb::RGB_TAG::GREENBLUE>().to_file(LR"(.\image_r.bmp)");
    image.copy().remove_colour<rgb::RGB_TAG::REDGREEN>().to_file(LR"(.\image_b.bmp)");
    image.copy().remove_colour<rgb::RGB_TAG::REDBLUE>().to_file(LR"(.\image_g.bmp)");

    image.copy().vflip().to_file(LR"(./vflipped.bmp)");

    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::AVERAGE>().to_file(LR"(./average.bmp)");
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::WEIGHTED_AVERAGE>().to_file(LR"(./weighted_average.bmp)");
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::BINARY>().to_file(LR"(./binary.bmp)");
    image.copy().to_blacknwhite<rgb::BW_TRANSFORMATION::LUMINOSITY>().to_file(LR"(./luminosity.bmp)");

    return EXIT_SUCCESS;
}
