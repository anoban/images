#include <algorithm>
#include <helpers>

#include <gtest/gtest.h>

TEST(MISC, TO_UNDERLYING) {
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

TEST(MISC, IS_IN) {
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

static constexpr char ASCII_UPPERCASE[] { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                          'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

static constexpr char ASCII_LOWERCASE[] { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                          'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

static constexpr char ASCII_LETTERS[] { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

static constexpr char ASCII_PRINTABLE[] { '0',  '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',  'b',  'c',  'd',    'e',   'f', 'g',
                                          'h',  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',  's',  't',  'u',    'v',   'w', 'x',
                                          'y',  'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',  'J',  'K',  'L',    'M',   'N', 'O',
                                          'P',  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  '!',  '"',  '#',    '$',   '%', '&',
                                          '\'', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';',  '<',  '=',  '>',    '?',   '@', '[',
                                          '\"', ']', '^', '_', '`', '{', '|', '}', '~', ' ', '\t', '\n', '\r', '\x0b', '\x0c' };

TEST(ASCII, IS_ALPHABET) {
    EXPECT_TRUE(std::all_of(std::cbegin(ASCII_UPPERCASE), std::cend(ASCII_UPPERCASE), ascii::is_alphabet));
    EXPECT_TRUE(std::all_of(std::cbegin(ASCII_LOWERCASE), std::cend(ASCII_LOWERCASE), ascii::is_alphabet));
    EXPECT_TRUE(std::all_of(std::cbegin(ASCII_LETTERS), std::cend(ASCII_LETTERS), ascii::is_alphabet));

    EXPECT_TRUE(std::any_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_alphabet));
    EXPECT_FALSE(std::all_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_alphabet));
}

TEST(ASCII, IS_ALPHABETARRAY) {
    EXPECT_TRUE(ascii::is_alphabet_array(ASCII_UPPERCASE));
    EXPECT_TRUE(ascii::is_alphabet_array(ASCII_LOWERCASE));
    EXPECT_TRUE(ascii::is_alphabet_array(ASCII_LETTERS));
    EXPECT_FALSE(ascii::is_alphabet_array(ASCII_PRINTABLE));
}

TEST(ASCII, IS_UPPERCASE) {
    EXPECT_TRUE(std::all_of(std::cbegin(ASCII_UPPERCASE), std::cend(ASCII_UPPERCASE), ascii::is_uppercase));
    EXPECT_FALSE(std::any_of(std::cbegin(ASCII_LOWERCASE), std::cend(ASCII_LOWERCASE), ascii::is_uppercase));

    EXPECT_TRUE(std::any_of(std::cbegin(ASCII_LETTERS), std::cend(ASCII_LETTERS), ascii::is_uppercase));
    EXPECT_FALSE(std::all_of(std::cbegin(ASCII_LETTERS), std::cend(ASCII_LETTERS), ascii::is_uppercase));

    EXPECT_TRUE(std::any_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_uppercase));
    EXPECT_FALSE(std::all_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_uppercase));
}

TEST(ASCII, IS_LOWERCASE) {
    EXPECT_TRUE(std::all_of(std::cbegin(ASCII_LOWERCASE), std::cend(ASCII_LOWERCASE), ascii::is_lowercase));
    EXPECT_FALSE(std::any_of(std::cbegin(ASCII_UPPERCASE), std::cend(ASCII_UPPERCASE), ascii::is_lowercase));

    EXPECT_TRUE(std::any_of(std::cbegin(ASCII_LETTERS), std::cend(ASCII_LETTERS), ascii::is_lowercase));
    EXPECT_FALSE(std::all_of(std::cbegin(ASCII_LETTERS), std::cend(ASCII_LETTERS), ascii::is_lowercase));

    EXPECT_TRUE(std::any_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_lowercase));
    EXPECT_FALSE(std::all_of(std::cbegin(ASCII_PRINTABLE), std::cend(ASCII_PRINTABLE), ascii::is_lowercase));
}
