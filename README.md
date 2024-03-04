# ___BMP___

```C
#include <bmp.h>
#include <heapapi.h>
#include <stdlib.h>

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) const wchar_t* const argv[]) {
    const HANDLE64 hProcHeap = GetProcessHeap();
    bmp_t          image;
    for (int64_t i = 1; i < argc; ++i) {
        image = BmpRead(argv[i]);
        // do something
        HeapFree(hProcHeap, 0, image.pixels);
    }
    return EXIT_SUCCESS;
}

```


----------------
___References:___

- `Compressed Image File Formats: JPEG, PNG, GIF, XBM, BMP - John Miano (1999) ACM Press/Addison-Wesley Publishing Co`.

- `PNG: The Definitive Guide. - Greg Roelofs and Richard Koman. (1999) O'Reilly Associates, Inc., USA`.

- `The Data Compression Book, Second Edition, Mark Nelson and Jean-Loop Gailly. 1995. M&T Books, New York, NY`

- https://koushtav.me/jpeg/tutorial/2017/11/25/lets-write-a-simple-jpeg-library-part-1/

- https://koushtav.me/jpeg/tutorial/c++/decoder/2019/03/02/lets-write-a-simple-jpeg-library-part-2/

----------------
