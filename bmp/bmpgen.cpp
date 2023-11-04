#include "bmp.hpp"

int main(void) {
    auto img { bmp::bmp::gengradient(4120, 7680) };
    img.serialize(L"./synth.bmp");

    auto imgsq { bmp::bmp::gengradient(1024, 1024) };
    imgsq.serialize(L"./synth_sq.bmp");

    auto imgsqg { bmp::bmp::gengradient(8192, 8192) };
    imgsqg.serialize(L"./synth_sqg.bmp");

    return EXIT_SUCCESS;
}