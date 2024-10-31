#include <bitmap>

#include <gtest/gtest.h>

TEST(BITMAP, FROM_FILE) {
    bitmap elsa { LR"(./castle.bmp)" };
    elsa.to_blacknwhite(BW_TRANSFORMATION::AVERAGE);
    elsa.to_file(LR"(./bw.bmp)");
}
