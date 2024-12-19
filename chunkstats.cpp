#define __INTERNAL
#include <algorithm>
#include <vector>

#include <_imageio.hpp>

template<unsigned long long length>
[[nodiscard]] constexpr bool is_same(_In_ const unsigned char* const bytestream, _In_ const char (&name)[length]) noexcept {
    for (unsigned long i = 0; i < (length - 1 /* offsetting for the null terminator */); ++i)
        if (bytestream[i] != name[i]) return false; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic, bugprone-signed-char-misuse)
    return true;
}

int wmain(_In_opt_ const int argc, _In_opt_ const wchar_t* const argv[]) { // NOLINT(modernize-avoid-c-arrays)
    if (argc == 1) {
        ::fputws(L"No file paths provided to the programme\n", stderr);
        return EXIT_FAILURE;
    }

    const unsigned char* filebuffer {};
    unsigned long        filesize {};
    unsigned long long   ihdr_count {}, idat_count {}, plte_count {}, iend_count {}; // NOLINT(readability-isolate-declaration)

    for (long long i = 1; i < argc; ++i) {
        filebuffer = internal::open(argv[i], filesize); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (!filebuffer) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-vararg)
            ::fwprintf_s(stderr, L"File buffer is empty, skipping image %s\n", argv[i]);
            continue;
        }

        for (long long j = 0; j < filesize - 4 /* to prevent buffer overuns */; ++j) {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (::is_same(filebuffer + j, "IHDR")) ihdr_count++;
            if (::is_same(filebuffer + j, "IDAT")) idat_count++;
            if (::is_same(filebuffer + j, "PLTE")) plte_count++;
            if (::is_same(filebuffer + j, "IEND")) iend_count++;
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        delete[] filebuffer;

        ::wprintf_s( // NOLINT(cppcoreguidelines-pro-type-vararg)
            L"%20s has %2llu IHDR, %2llu PLTE, %5llu IDAT and %2llu IEND chunks in it\n",
            argv[i], // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ihdr_count,
            plte_count,
            idat_count,
            iend_count
        );
        ihdr_count = idat_count = plte_count = iend_count = 0;
    }

    return EXIT_SUCCESS;
}
