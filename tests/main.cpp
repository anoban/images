#pragma warning(disable : 4625 4626 5026 5027 5045 4668) // gtest
#include <gtest/gtest.h>

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
