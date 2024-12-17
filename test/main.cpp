#include <gtest/gtest.h>

int main() {
    ::srand(::time(nullptr));

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
