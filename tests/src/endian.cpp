#include <vector>

#include <endian.h>

// clang-format off
#include <_utilities.hpp>
#include <gtest/gtest.h>
// clang-format on

struct EndianTest : public testing::Test {
        static constexpr unsigned long long RANDOM_NUMBER_BUFFER_LENGTH { 0xFFF * sizeof(unsigned long long) };
        std::vector<unsigned char>          buffer;

        virtual void SetUp() noexcept override {
            buffer.resize(RANDOM_NUMBER_BUFFER_LENGTH);
            for (unsigned long long i = 0; i < RANDOM_NUMBER_BUFFER_LENGTH; ++i) buffer[i] = static_cast<unsigned char>(::rand());
        }
};

TEST_F(EndianTest, u16_from_be_bytes) {
    for (unsigned long long i = 0; i <= RANDOM_NUMBER_BUFFER_LENGTH - sizeof(unsigned short) /* this is to avoid buffer overruns */; ++i)
        EXPECT_EQ(utilities::endian::u16_from_be_bytes(buffer.data() + i), ::__bswap_16(*reinterpret_cast<const unsigned short*>(buffer.data() + i)));
}

TEST_F(EndianTest, u32_from_be_bytes) {
    for (unsigned long long i = 0; i <= RANDOM_NUMBER_BUFFER_LENGTH - sizeof(unsigned); ++i)
        EXPECT_EQ(utilities::endian::u32_from_be_bytes(buffer.data() + i), ::__bswap_32(*reinterpret_cast<const unsigned*>(buffer.data() + i)));
}

TEST_F(EndianTest, u64_from_be_bytes) {
    for (unsigned long long i = 0; i <= RANDOM_NUMBER_BUFFER_LENGTH - sizeof(unsigned long long); ++i)
        EXPECT_EQ(utilities::endian::u64_from_be_bytes(buffer.data() + i), ::__bswap_64(*reinterpret_cast<const unsigned long long*>(buffer.data() + i)));
}
