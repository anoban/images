#include <bitmap>

#include <gtest/gtest.h>

TEST(BITMAP, FROM_FILE) {
    bitmap image { LR"(./black.bmp)" };
    image.make_blacknwhite(BW_TRANSFORMATION::AVERAGE).to_file(LR"(./bw_average.bmp)");
    image.make_blacknwhite(BW_TRANSFORMATION::WEIGHTED_AVERAGE).to_file(LR"(./bw_weighted_average.bmp)");
    image.info();
}
