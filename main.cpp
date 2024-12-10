#include <canvas>
#include <chrono>
#include <cmaps>
#include <format>
#include <random>

int main() {
    std::mt19937_64 reng { static_cast<unsigned long long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };

    canvas          board { 1080, 1920 };
    board.fill(RGBQUAD { static_cast<unsigned char>(reng() % std::numeric_limits<unsigned char>::max()) /* B */,
                         static_cast<unsigned char>(reng() % std::numeric_limits<unsigned char>::max()) /* G */,
                         static_cast<unsigned char>(reng() % std::numeric_limits<unsigned char>::max()) /* R */,
                         0XFF });
    board.to_file(LR"(yellow.bmp)");

    board.mandelbrot(colourmaps::JET);
    board.to_file(LR"(./mandelbrot_jet.bmp)");

    // ::tricorn(board, colourmaps::JET);
    // board.to_file(LR"(./tricorn_jet.bmp)");

    return EXIT_SUCCESS;
}
