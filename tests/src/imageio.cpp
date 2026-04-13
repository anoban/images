// clang-format off
#include <_utilities.hpp>
#include <_imageio.hpp>
#include <gtest/gtest.h>
// clang-format on

TEST(IO, imopen) {
    long readbytes {};

    const unsigned char* buffer = io::imopen("R(./media/nonexistent.bin)", readbytes);
    ASSERT_EQ(readbytes, 0);
    ASSERT_EQ(buffer, nullptr);

    buffer = io::imopen("R(./media/janeeyre.txt)", readbytes);
    ASSERT_EQ(readbytes, 0);
    ASSERT_EQ(buffer, nullptr);

    buffer = io::imopen("R(./media/warandpeace.txt)", readbytes);
    ASSERT_EQ(readbytes, 0);
    ASSERT_EQ(buffer, nullptr);

    buffer = io::imopen("R(./media/image.jpg)", readbytes);
    ASSERT_EQ(readbytes, 0);
    ASSERT_EQ(buffer, nullptr);

    buffer = io::imopen("R(./media/python313.dll)", readbytes);
    ASSERT_EQ(readbytes, 0);
    ASSERT_EQ(buffer, nullptr);
}

TEST(IO, imwrite) {
    //
}
