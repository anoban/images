#include <canvas>
#include <ico>

int wmain() {
    bitmap image { LR"(./black.bmp)" };
    canvas board { std::move(image) };

    board.fill({ 0, 255, 255, 0xFF });
    board.to_file(LR"(yellow.bmp)");

    return EXIT_SUCCESS;
}
