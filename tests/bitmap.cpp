#include <bitmap>

#include <gtest/gtest.h>

TEST(BITMAP, FROM_FILE) {
    bitmap elsa { LR"(./black.bmp)" };
    elsa.make_blacknwhite(BW_TRANSFORMATION::AVERAGE).to_file(LR"(./bw_average.bmp)");
    elsa.make_blacknwhite(BW_TRANSFORMATION::WEIGHTED_AVERAGE).to_file(LR"(./bw_weighted_average.bmp)");
}
