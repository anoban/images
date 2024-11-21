#include <algorithm>
#include <helpers>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

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

static constexpr std::array<unsigned char, 1000> bytes {
    167, 94,  14,  194, 22,  111, 153, 190, 246, 199, 160, 99,  51,  232, 51,  77,  162, 67,  82,  207, 137, 194, 103, 148, 176, 163, 240,
    53,  136, 75,  138, 42,  169, 4,   95,  9,   21,  239, 191, 223, 97,  68,  155, 88,  158, 226, 189, 18,  237, 104, 6,   169, 76,  126,
    212, 83,  246, 98,  41,  146, 10,  71,  124, 42,  170, 194, 248, 221, 179, 123, 40,  209, 240, 123, 69,  203, 57,  228, 204, 217, 104,
    176, 209, 54,  144, 237, 179, 151, 61,  236, 37,  223, 22,  8,   62,  78,  213, 130, 249, 223, 111, 243, 67,  65,  32,  233, 222, 183,
    212, 24,  242, 18,  37,  165, 60,  161, 207, 2,   209, 48,  119, 29,  213, 114, 106, 211, 153, 119, 4,   131, 62,  140, 205, 14,  114,
    92,  240, 147, 227, 215, 50,  18,  105, 179, 93,  96,  94,  125, 224, 42,  143, 240, 128, 62,  48,  81,  249, 243, 208, 49,  10,  196,
    125, 132, 144, 221, 132, 19,  142, 200, 238, 155, 123, 226, 115, 29,  85,  143, 78,  70,  71,  163, 195, 210, 185, 229, 99,  5,   249,
    50,  209, 200, 142, 37,  175, 237, 57,  232, 225, 158, 149, 18,  55,  176, 154, 215, 149, 81,  133, 170, 23,  124, 135, 97,  161, 181,
    136, 4,   37,  138, 161, 244, 183, 188, 199, 47,  168, 48,  235, 129, 89,  136, 180, 91,  179, 37,  245, 17,  220, 119, 168, 100, 233,
    162, 85,  66,  148, 52,  92,  39,  122, 161, 227, 95,  181, 4,   101, 113, 212, 90,  136, 184, 220, 103, 182, 79,  156, 128, 148, 136,
    15,  118, 6,   141, 103, 8,   170, 209, 135, 33,  228, 218, 162, 97,  31,  111, 140, 84,  73,  134, 178, 59,  166, 58,  110, 107, 103,
    224, 8,   33,  185, 88,  121, 208, 162, 200, 134, 188, 183, 92,  60,  119, 51,  212, 44,  171, 52,  5,   60,  158, 73,  194, 110, 103,
    141, 242, 114, 16,  60,  61,  76,  248, 146, 110, 44,  214, 117, 195, 74,  208, 45,  155, 142, 58,  38,  18,  132, 247, 201, 46,  92,
    86,  89,  197, 124, 205, 159, 100, 160, 171, 201, 30,  58,  215, 85,  43,  38,  56,  232, 130, 36,  237, 4,   43,  105, 23,  85,  59,
    51,  3,   98,  73,  188, 125, 191, 152, 123, 177, 26,  162, 189, 33,  77,  131, 230, 217, 106, 130, 48,  210, 130, 145, 81,  40,  143,
    65,  213, 204, 171, 19,  201, 187, 224, 18,  147, 58,  61,  139, 87,  201, 51,  162, 84,  132, 136, 248, 108, 0,   177, 98,  92,  190,
    99,  247, 50,  119, 125, 123, 142, 119, 149, 182, 240, 119, 162, 76,  66,  196, 179, 162, 237, 140, 86,  140, 73,  95,  32,  23,  112,
    93,  190, 1,   178, 144, 87,  62,  0,   132, 84,  131, 204, 0,   27,  232, 244, 71,  198, 59,  16,  8,   180, 137, 36,  172, 43,  206,
    138, 134, 2,   213, 24,  22,  143, 180, 145, 221, 222, 155, 235, 7,   52,  181, 66,  104, 219, 20,  199, 56,  41,  231, 154, 49,  186,
    169, 157, 54,  192, 247, 215, 248, 213, 106, 79,  188, 123, 189, 108, 20,  48,  226, 136, 103, 98,  218, 236, 165, 31,  112, 118, 34,
    21,  123, 196, 199, 103, 236, 149, 176, 122, 157, 198, 13,  126, 213, 113, 81,  71,  121, 52,  241, 112, 196, 14,  232, 21,  144, 153,
    115, 130, 37,  237, 122, 212, 207, 233, 153, 77,  60,  88,  113, 171, 160, 113, 178, 202, 196, 114, 35,  172, 1,   97,  57,  147, 52,
    58,  21,  107, 183, 202, 18,  57,  43,  165, 61,  138, 229, 177, 0,   232, 27,  23,  10,  69,  35,  100, 144, 180, 191, 192, 36,  107,
    149, 201, 246, 1,   205, 137, 118, 127, 118, 67,  213, 166, 172, 45,  195, 145, 191, 11,  189, 102, 206, 12,  161, 203, 187, 160, 11,
    68,  240, 175, 39,  113, 89,  111, 124, 31,  60,  165, 160, 56,  171, 58,  106, 225, 192, 100, 171, 133, 182, 193, 59,  98,  84,  106,
    81,  86,  162, 167, 204, 69,  238, 164, 141, 214, 3,   161, 247, 176, 50,  118, 238, 200, 200, 12,  123, 118, 199, 56,  240, 1,   200,
    95,  223, 135, 117, 229, 77,  142, 142, 220, 151, 112, 114, 17,  142, 93,  201, 72,  92,  228, 133, 245, 140, 17,  230, 20,  244, 237,
    190, 226, 13,  105, 82,  96,  42,  40,  154, 38,  114, 188, 53,  206, 194, 215, 99,  95,  89,  90,  8,   132, 5,   177, 85,  152, 21,
    163, 116, 129, 127, 22,  57,  242, 105, 229, 18,  237, 142, 184, 1,   13,  125, 29,  153, 217, 102, 195, 249, 35,  178, 82,  73,  237,
    204, 224, 227, 196, 185, 181, 121, 164, 119, 176, 155, 201, 184, 130, 141, 147, 16,  165, 166, 146, 234, 79,  137, 194, 154, 119, 62,
    157, 169, 23,  36,  119, 188, 110, 185, 78,  145, 164, 121, 112, 23,  220, 147, 227, 236, 219, 149, 37,  128, 45,  67,  232, 239, 121,
    98,  145, 202, 187, 82,  72,  233, 116, 190, 41,  163, 46,  115, 212, 91,  79,  211, 202, 203, 61,  63,  37,  241, 144, 46,  45,  116,
    60,  89,  134, 177, 69,  197, 174, 21,  128, 96,  87,  144, 69,  50,  157, 63,  72,  113, 87,  207, 214, 133, 1,   225, 221, 124, 58,
    209, 225, 131, 101, 218, 224, 133, 100, 209, 11,  54,  58,  204, 211, 38,  68,  6,   209, 166, 48,  235, 57,  165, 181, 238, 74,  155,
    140, 64,  138, 44,  135, 125, 4,   86,  30,  197, 76,  186, 204, 37,  65,  84,  37,  114, 126, 102, 204, 13,  126, 148, 195, 203, 127,
    14,  39,  41,  76,  23,  119, 49,  99,  237, 64,  198, 191, 232, 153, 25,  25,  139, 141, 104, 202, 45,  184, 106, 202, 239, 237, 97,
    237, 141, 167, 242, 38,  93,  230, 222, 56,  165, 15,  79,  240, 12,  34,  228, 114, 119, 85,  225, 28,  223, 131, 112, 118, 46,  188,
    203
};

TEST(ENDIAN, USHORT_FROM_BE_BYTES) {
    for (unsigned long long i = 0; i <= bytes.size() - sizeof(unsigned short); ++i)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EXPECT_EQ(endian::ushort_from_be_bytes(bytes.data() + i), ::ntohs(*reinterpret_cast<const unsigned short*>(bytes.data() + i)));
}

TEST(ENDIAN, ULONG_FROM_BE_BYTES) {
    for (unsigned long long i = 0; i <= bytes.size() - sizeof(unsigned long); ++i)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EXPECT_EQ(endian::ulong_from_be_bytes(bytes.data() + i), ::ntohl(*reinterpret_cast<const unsigned long*>(bytes.data() + i)));
}

TEST(ENDIAN, ULLONG_FROM_BE_BYTES) {
    for (unsigned long long i = 0; i <= bytes.size() - sizeof(unsigned long long); ++i)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EXPECT_EQ(endian::ullong_from_be_bytes(bytes.data() + i), ::ntohll(*reinterpret_cast<const unsigned long long*>(bytes.data() + i)));
}
