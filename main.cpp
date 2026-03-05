#include <bitmap>
#include <canvas>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-redundant-inline-specifier)

static inline const char* gsub(const char* const fpath, const char* const rstrip, const char* const replacement) noexcept {
    static constexpr unsigned long long MAXPATH { 0X200 };
    static char                         buffer[MAXPATH] {};

    ::memset(buffer, 0, MAXPATH);                                   // at every call, cleanup the buffer before any writes
    ::strcpy(buffer, fpath);                                        // copy the path over to the buffer
    char* const mark = const_cast<char*>(::strstr(buffer, rstrip)); // pointer to the char where our substring begins in the path
    // if we pass this to ::strcat() as is, it'll append the replacement after the null terminator, at the end of the path, not at this mark
    *mark            = 0; // first zero the delim and then pass it to ::strcat(), so the write starts at the specified mark
    ::strcat(mark, replacement);
    return buffer;
}

int main(const int argc, const char* const argv[]) {
    if (argc == 1) return EXIT_FAILURE; // expect the file name to be passed in argv

    for (int i = 1; i < argc; ++i) {
        ::canvas palette { ::bitmap { argv[i] } };
        if (!palette.to_blacknwhite<BW::AVERAGE>().to_file(::gsub(argv[i], ".bmp", "_bw.bmp"))) ::fputs("serialization filed!\n", stderr);
    }

    ::puts("done!");

    return EXIT_SUCCESS;
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,readability-redundant-inline-specifier)
