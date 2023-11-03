#include "bmp.hpp"

int main(void) {
    auto img { bmp::bmp::gengradient(2160, 3840) };
    img.serialize(L"./synth.bmp");
    return EXIT_SUCCESS;
}