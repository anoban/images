#include <canvas>
#include <ico>

int wmain() {
    icon_directory icon { LR"(./ico/intel.ico)" };
    std::wcout << icon;

    const auto image { icon.to_bitmap(0) };
    std::wcout << image;
    image.to_file(LR"(convert.bmp)");

    return EXIT_SUCCESS;
}
