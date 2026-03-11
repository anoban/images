#include <bitmap>
#include <canvas>

[[maybe_unused]] static inline const char* gsub(
    const char* const string, const char* const pattern, const char* const replacement
) noexcept {
    static constexpr unsigned long long MAXPATH { 0X200 };
    static char                         buffer[MAXPATH] {};
    ::memset(buffer, 0, MAXPATH);                                    // at every call, cleanup the buffer before any writes
    ::strcpy(buffer, string);                                        // copy the path over to the buffer
                                                                     // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    char* const mark = const_cast<char*>(::strstr(buffer, pattern)); // pointer to the char where our substring begins in the path
    // if we pass this to ::strcat() as is, it'll append the replacement at the null terminator, at the end of the path, not at this mark
    *mark            = 0; // first zero the delim and then pass it to ::strcat(), so the write starts at the specified mark
    ::strcat(mark, replacement);
    return buffer;
}

int main(const int argc, const char* const argv[]) {
    if (argc == 1) return EXIT_FAILURE; // expect the file name to be passed in argv

    for (int i = 1; i < argc; ++i) {
        ::canvas palette { ::bitmap { argv[i] } };
        // if (!palette.to_negative().to_file(::gsub(argv[i], ".bmp", "_neg.bmp"))) ::fputs("serialization filed!\n", stderr);
        ::puts(palette.to_text(palettes::extended, totext::arithmetic_average).c_str());
    }

    ::puts("done!");

    return EXIT_SUCCESS;
}
