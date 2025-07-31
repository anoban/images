pub mod colormaps {

    use crate::utilities::wingdi::RgbQuad;

    // colourmaps courtesy of https://github.com/ArashPartow/bitmap/blob/master/bitmap_image.hpp
    // his colourmaps use struct rgb_t { unsigned char red; unsigned char green; unsigned char blue; }; as the pixel type (RGB sequence)
    // hence they have been refactored to comply with WinGdi's RGBQUAD struct, which sequences colours in BGR order
    // contains nine colourmaps named YARG, VGA, PRISM, JET, HSV, HOT, GREY, COPPER and AUTUMN

    const CMAPSIZE: usize = 1000;
}
