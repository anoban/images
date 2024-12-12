#include <canvas>
#include <ico>

int wmain(_In_ int argc, _In_ const wchar_t* argv[]) {
    icondirectory icon { argv[1] };
    std::wcout << icon;
    const auto image { icon.to_bitmap() };
    std::wcout << image;
    // image.to_file(LR"(convert.bmp)");
    return EXIT_SUCCESS;
}
