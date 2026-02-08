#include <canvas>
#include <format>
#include <ico>
#include <png>

int wmain(const int argc, const wchar_t* const wargv[]) {
    if (argc != 2) return EXIT_FAILURE;

    unsigned long     size {};
    const auto* const pngstream { internal::open(LR"(./sweeney.png)", size) };
    std::wcout << size << L'\n';

    internal::IHDR ihdr { pngstream + 8 };
    std::wcout << ihdr;
    std::wcout << std::boolalpha << ihdr.is_valid() << L'\n';

    internal::IEND iend { pngstream + size - 12 };
    std::wcout << iend;
    std::wcout << std::boolalpha << iend.is_valid() << L'\n';

    delete[] pngstream;

    return EXIT_SUCCESS;
}
