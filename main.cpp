#include <canvas>
#include <ico>

int wmain(_In_ const int argc, _In_ const wchar_t** const argv) {
    // if (argc == 1) return EXIT_FAILURE;

    bitmap image { LR"(./black.bmp)" };
    canvas board { std::move(image) };

    board.fill({ 0x00, 0x0A, 0x0A, 0xFF });
    board.to_file(LR"(canvas.bmp)");

    return EXIT_SUCCESS;
}
