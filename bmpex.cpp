#include "bmp.hpp"

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) wchar_t* argv[]) {
    for (size_t i = 1; i < argc; ++i) {
        size_t nbytes {};
        try {
            auto img { bmp::bmp(argv[i]) };
            img.serialize(L"whoops.bmp");
        } catch (const std::exception& excpt) { ::fwprintf_s(stderr, L"%s: %S\n", argv[i], excpt.what()); }
    }
    return EXIT_SUCCESS;
}