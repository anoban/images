#include <canvas>
#include <format>
#include <ico>

int wmain() {
    // icon_directory icon { LR"(./ico/intel.ico)" };
    // std::wcout << icon;
    //
    // for (unsigned long i = 0; i < icon.resource_count(); ++i) {
    //     const auto image { icon.to_bitmap(i) };
    //     std::wcout << image;
    //     image.to_file(std::format(L"icon_{:03d}.bmp", i).c_str());
    // }

    canvas board { LR"(./guitar.bmp)" };
    std::wcout << board;
    board.resize_for_overwrite(2600, 1600).to_file(LR"(resized_for_overwrite.bmp)");

    return EXIT_SUCCESS;
}
