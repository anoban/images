#include <bitmap>
#include <canvas>
#include <format>

static inline const char* add_suffix(const char* const fpath, const char* const rstrip, const char* const replacement) noexcept {
    static constexpr long long MAX_PATH { 0X2FF };
    static char                buffer[MAX_PATH] {};
    ::memset(buffer, 0, MAX_PATH);
    ::strcpy(buffer, fpath);
    char* const delim = const_cast<char*>(::strstr(buffer, rstrip)); // the length of the string we actually need
    // if we pass delim to ::strcat as is, it'll append after the null terminator at the end, not at delim
    // first zero the delim and pass it to ::strcat
    *delim            = 0;
    ::strcat(delim, replacement);
    return buffer;
}

int main(const int argc, const char* const argv[]) {
    if (argc == 1) return EXIT_FAILURE; // expect the file name to be passed in argv

    for (int i = 1; i < argc; ++i) {
        ::canvas cv { ::bitmap { argv[i] } };
        if (!cv.to_blacknwhite<BW_TRANSFORMATION::AVERAGE>().to_file(::add_suffix(argv[i], ".bmp", "_bw.bmp")))
            ::fputs("serialization filed!\n", stderr);
    }

    ::puts("Done!");

    return EXIT_SUCCESS;
}
