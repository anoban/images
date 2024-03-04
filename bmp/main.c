#include <bmp.h>
#include <heapapi.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    const HANDLE64 hProcHeap = GetProcessHeap();
    for (int64_t i = 1; i < argc; ++i) const bmp_t image = BmpRead(argv[i]);
    return EXIT_SUCCESS;
}
