#include "bmp.hpp"

int main(void) {
    auto img { bmp::bmp::gengradient() };
    img.serialize(L"./synth.bmp");
    return EXIT_SUCCESS;
}