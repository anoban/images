#include <algorithm>
#include <png>
#include <vector>

#include <_imageio.hpp>

template<unsigned long long length> constexpr bool is_same(const unsigned char* const bytestream, const char (&name)[length]) noexcept {
    for (unsigned long i = 0; i < (length - 1 /* offsetting for the null terminator */); ++i)
        if (bytestream[i] != name[i]) return false;
    return true;
}

int main() {
    const unsigned char* filebuffer {};
    long                 filesize {};
    unsigned long long   ihdr_count {}, idat_count {}, plte_count {}, iend_count {}; // NOLINT(readability-isolate-declaration)

    filebuffer = internal::imopen(R"(/home/anoban/Downloads/armenia_ani_khachikyan_hayk_photography-wallpaper-5120x3200.png)", filesize);
    if (!filebuffer) {
        ::fputs("File buffer is empty, skipping image %s\n", stderr);
        return EXIT_FAILURE;
    }

    for (long long i = 0; i < filesize - 4 /* to prevent buffer overuns */; ++i) {
        if (::is_same(filebuffer + i, "IHDR")) {
            std::cout << critical::ihdr(filebuffer + i - 4);
            ihdr_count++;
        }
        if (::is_same(filebuffer + i, "IDAT")) idat_count++;
        if (::is_same(filebuffer + i, "PLTE")) plte_count++;
        if (::is_same(filebuffer + i, "IEND")) {
            iend_count++;
            std::cout << critical::iend(filebuffer + i - 4);
        }
    }

    delete[] filebuffer;

    ::printf("Image has %2llu IHDR, %2llu PLTE, %5llu IDAT and %2llu IEND chunks in it\n", ihdr_count, plte_count, idat_count, iend_count);
    ihdr_count = idat_count = plte_count = iend_count = 0;

    return EXIT_SUCCESS;
}
