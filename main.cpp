#include <canvas>
#include <chrono>
#include <cmaps>
#include <format>
#include <random>

int main() {
    std::mt19937_64 rand_engine { static_cast<unsigned long long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };
    std::uniform_int_distribution<long> runif { 2500, 3000 };

    canvas                              board { 1080, 1920 };

    board.mandelbrot(colourmaps::JET);
    board.to_file(LR"(./mandelbrot_jet.bmp)");

    // ::tricorn(board, colourmaps::JET);
    // board.to_file(LR"(./tricorn_jet.bmp)");

    return EXIT_SUCCESS;
}
