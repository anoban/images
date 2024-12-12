#define __TEST__
#include <canvas>
#include <ico>

int wmain(_In_ int argc, _In_ const wchar_t* argv[]) {
    icondirectory icon { argv[1] };
    std::wcout << icon;
    RGBQUAD* pixels {};

    for (long i = 0; i < icon.directory.idCount; ++i) {
        pixels = reinterpret_cast<RGBQUAD*>(icon.buffer + icon.entries.at(i).dwImageOffset);
        for (long j = 0; j < icon.entries.at(i).bWidth * icon.entries.at(i).bHeight; ++j) {
            //
            std::wcout << pixels[j].rgbBlue << L' ' << pixels[j].rgbGreen << L' ' << pixels[j].rgbRed << L' ' << pixels[j].rgbReserved
                       << L'\n';
        }
    }
    // const auto image { icon.to_bitmap() };
    // std::wcout << image;
    // image.to_file(LR"(convert.bmp)");
    return EXIT_SUCCESS;
}
