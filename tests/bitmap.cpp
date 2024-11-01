#include <bitmap>

#include <gtest/gtest.h>

inline namespace __colour_removers {

    TEST(COLOUR_REMOVER, REMOVE) {
        RGBQUAD pixel { 255, 255, 255, 255 };

        colour_removers::remove<RGB_TAG::BLUE> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 0, 255, 255, 255 })); // the surrounding parenthesis is important
        colour_removers::remove<RGB_TAG::GREEN> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 0, 0, 255, 255 }));
        colour_removers::remove<RGB_TAG::RED> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 0, 0, 0, 255 }));

        pixel = { 255, 255, 255, 255 };
        colour_removers::remove<RGB_TAG::GREENBLUE> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 0, 0, 255, 255 }));
        pixel = { 255, 255, 255, 255 };
        colour_removers::remove<RGB_TAG::REDBLUE> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 0, 255, 0, 255 }));
        pixel = { 255, 255, 255, 255 };
        colour_removers::remove<RGB_TAG::REDGREEN> {}(pixel);
        EXPECT_EQ(pixel, (RGBQUAD { 255, 0, 0, 255 }));
    }

} // namespace __colour_removers

TEST(BITMAP, FROM_FILE) {
    bitmap image { LR"(./black.bmp)" };
    image.make_blacknwhite(BW_TRANSFORMATION::AVERAGE).to_file(LR"(./bw_average.bmp)");
    image.make_blacknwhite(BW_TRANSFORMATION::WEIGHTED_AVERAGE).to_file(LR"(./bw_weighted_average.bmp)");
    image.info();
}
