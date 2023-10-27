#include "bmp.hpp"

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) wchar_t* argv[]) {
    for (size_t i = 1; i < argc; ++i) {
        size_t nbytes {};
        auto   buffer { bmp::open(argv[i], &nbytes) };
        ::wprintf_s(L"%s is %zu bytes\n", argv[i], nbytes);
    }
    return EXIT_SUCCESS;
}