#define __INTERNAL
#include <algorithm>
#include <vector>

#include <_imageio.hpp>

template<unsigned long long length>
[[nodiscard]] constexpr bool is_same(_In_ const unsigned char* const bytestream, _In_ const char (&name)[length]) noexcept {
    for (unsigned long i = 0; i < (length - 1); ++i)
        if (bytestream[i] != name[i]) return false;
    return true;
}

int wmain(_In_opt_ int argc, _In_opt_ const wchar_t* const argv[]) {
    if (argc == 1) {
        ::fputws(L"No file paths provided to the programme\n", stderr);
        return EXIT_FAILURE;
    }

    const unsigned char* filebuffer {};
    unsigned long        filesize {};

    for (long long i = 1; i < argc; ++i) {
        std::vector<unsigned long long> idat_offsets;
        filebuffer = internal::open(argv[i], filesize);
        if (!filebuffer) {
            ::fputws(L"File buffer is empty\n", stderr);
            continue;
        }

        for (long long j = 0; j < filesize - 4; ++j)
            if (::is_same(filebuffer + j, "PLTE")) idat_offsets.push_back(j);

        delete[] filebuffer;

        ::wprintf_s(L"%20s has %10llu PLTE chunks in it\n", argv[i], idat_offsets.size());
        // for (unsigned long long i = 0; i < idat_offsets.size(); ++i) ::wprintf_s(L"%10llu\n", idat_offsets.at(i));
    }

    return EXIT_SUCCESS;
}
