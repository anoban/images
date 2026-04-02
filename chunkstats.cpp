#include <algorithm>
#include <png>
#include <vector>

#include <_imageio.hpp>

template<unsigned long long length> constexpr bool is_same(const unsigned char* const bytestream, const char (&name)[length]) noexcept {
    for (unsigned long i = 0; i < (length - 1 /* offsetting for the null terminator */); ++i)
        if (bytestream[i] != name[i]) return false;
    return true;
}

int main(const int argc, const char* const argv[]) { // NOLINT(modernize-avoid-c-arrays)
    if (argc == 1) {
        ::fputs("No file paths provided to the programme\n", stderr);
        return EXIT_FAILURE;
    }

    const unsigned char* filebuffer {};
    long                 filesize {};
    unsigned long long   ihdr_count {}, idat_count {}, plte_count {}, iend_count {}; // NOLINT(readability-isolate-declaration)

    for (long long i = 1; i < argc; ++i) {
        filebuffer = internal::imopen(argv[i], filesize);
        if (!filebuffer) {
            ::fprintf(stderr, "File buffer is empty, skipping image %s\n", argv[i]);
            continue;
        }

        for (long long j = 0; j < filesize - 4 /* to prevent buffer overuns */; ++j) {
            if (::is_same(filebuffer + j, "IHDR")) {
                std::cout << critical::ihdr(filebuffer + j - 8);
                ihdr_count++;
            }
            if (::is_same(filebuffer + j, "IDAT")) idat_count++;
            if (::is_same(filebuffer + j, "PLTE")) plte_count++;
            if (::is_same(filebuffer + j, "IEND")) {
                iend_count++;
                std::cout << critical::iend(filebuffer + j - 8);
            }
        }

        delete[] filebuffer;

        ::printf(
            "%20s has %2llu IHDR, %2llu PLTE, %5llu IDAT and %2llu IEND chunks in it\n",
            argv[i],
            ihdr_count,
            plte_count,
            idat_count,
            iend_count
        );
        ihdr_count = idat_count = plte_count = iend_count = 0;
    }

    return EXIT_SUCCESS;
}
