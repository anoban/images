#include <canvas>
#include <format>
#include <ico>
#include <png>

int wmain() {
    unsigned long     size {};
    const auto* const pngstream { internal::open(LR"(./sweeney.png)", size) };
    std::wcout << size << L'\n';

    internal::basic_chunk ihdr { pngstream + 8 };
    std::wcout << ihdr;

    delete[] pngstream;

    return EXIT_SUCCESS;
}
