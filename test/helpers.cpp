#include <helpers>

#include <gtest/gtest.h>

TEST(HELPERS, TO_UNDERLYING) {
    enum class TENS : unsigned long { ZERO = 0, TEN = 10, HUNDRED = 100, THOUSAND = 1000, TENTHOUSAND = 10'000 };
    enum class DAYS : long long { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY };

    EXPECT_EQ((::to_underlying(TENS::ZERO)), 0);
    EXPECT_EQ((::to_underlying(TENS::TEN)), 10);
    EXPECT_EQ((::to_underlying(TENS::HUNDRED)), 100);
    EXPECT_EQ((::to_underlying(TENS::THOUSAND)), 1000);
    EXPECT_EQ((::to_underlying(TENS::TENTHOUSAND)), 10'000);

    EXPECT_EQ((::to_underlying(DAYS::MONDAY)), 0);
    EXPECT_EQ((::to_underlying(DAYS::TUESDAY)), 1);
    EXPECT_EQ((::to_underlying(DAYS::WEDNESDAY)), 2);
    EXPECT_EQ((::to_underlying(DAYS::THURSDAY)), 3);
    EXPECT_EQ((::to_underlying(DAYS::FRIDAY)), 4);
    EXPECT_EQ((::to_underlying(DAYS::SATURDAY)), 5);
    EXPECT_EQ((::to_underlying(DAYS::SUNDAY)), 6);
}

TEST(HELPERS, ISIN) {
    EXPECT_TRUE(::is_in(10, 10U));
    EXPECT_TRUE(::is_in(0, 0L, 1, 2LL, 3LLU, 4, 'Z', 6, 7, 8, 9, 10));
    EXPECT_TRUE(::is_in(10, 0L, 1, 2LLU, 3, '{', 5, '%', 9LL, 10U));
    EXPECT_TRUE(::is_in(8, 0, 5, 6, '-', 8, 9, 10));
    EXPECT_FALSE(::is_in(7, 'X'));
    EXPECT_FALSE(::is_in(31, '[', 1, 2, '@', 4, 5LL, 6, 7, 8U, 9LU, 10));
    EXPECT_FALSE(::is_in(4LL, 0, 1U, 9, 10LU));
}

TEST(RGB, RGBQUAD_EQUALITY_OPERATORS) {
    EXPECT_EQ((RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }), (RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }));
    EXPECT_EQ((RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }), (RGBQUAD { 0xAE, 0x11, 0xFF, 0xAA }));
    EXPECT_NE((RGBQUAD { 0x1E, 0x11, 0xFF, 0xD0 }), (RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }));
    EXPECT_NE((RGBQUAD { 0xAE, 0xBE, 0xFF, 0xD0 }), (RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }));
    EXPECT_NE((RGBQUAD { 0xAE, 0x11, 0x98, 0xA0 }), (RGBQUAD { 0xAE, 0x11, 0xFF, 0xD0 }));
}

struct RgbQuadFixture : public testing::Test {
        RGBQUAD pixel;

        constexpr void SetUp() noexcept override {
            pixel = { .rgbBlue { 0xFF }, .rgbGreen { 0xFF }, .rgbRed { 0xFF }, .rgbReserved { 0xFF } };
        }

        constexpr void TearDown() noexcept override {
            pixel = { .rgbBlue { 0x00 }, .rgbGreen { 0x00 }, .rgbRed { 0x00 }, .rgbReserved { 0x00 } };
        }
};
