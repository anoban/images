#include <ico>

int wmain(_In_ const int argc, _In_ const wchar_t** const argv) {
    if (argc == 1) return EXIT_FAILURE;

    for (int i = 1; i < argc; ++i) {
        const icon_directory image { argv[i] };
        std::wcout << argv[i] << "  " << image.image_count() << L'\n';
    }

    return EXIT_SUCCESS;
}
