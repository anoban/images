#pragma once
#ifndef __BMP_H__
    #define __BMP_H__
    #define _AMD64_ // architecture
    #define WIN32_LEAN_AND_MEAN
    #define WIN32_EXTRA_MEAN

    #include <assert.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>

    // these data structures are implemented for learning purpose, but the functions are designed to operate with WinGdi structs
    // not these implementations
    #ifndef __USE_HANDROLLED_BMP_STRUCTS__ //  DON'T UNLESS ABSOLUTELY NECESSARY
        #include <windef.h>
        #include <wingdi.h>
    #endif // !__USE_HANDROLLED_BMP_STRUCTS__

    #include <errhandlingapi.h>
    #include <handleapi.h>
    #include <heapapi.h>
    #pragma comment(lib, "Gdi32.lib")

    #include <../imageio.h>

/*
   BMP format supports 1, 4, 8, 16, 24 and 32 bits per pixel.
   Even though Windows BMP format supports simple run length compression for 4 or 8 bits per pixel, it's rarely useful since
   it can only be used with large pixel blocks of identical colours.
   Multibyte integers in Windows BMP are stored LSB first (LE byte  order)
*/

// Since BMP format originted in Microsoft, Windows SDK comes pre-packed with almost all necessary data structures and routines
// needed to read in and process

// every Windows BMP begins with a BITMAPFILEHEADER struct
// this helps in recognizing the file format as .bmp
// the first two bytes will be 'B', 'M'

// all these data structures are provided in wingdi.h, so let's just use them instead of reinventing the wheel.
    #ifdef __USE_HANDROLLED_BMP_STRUCTS__ // JUST DON'T

        #pragma region _HANDROLLED_STRUCTS_
    // #pragma pack directive is a risky business, will likely impede gratuitous runtime performance penalties
        #pragma pack(push, 1)
typedef struct {
        uint8_t  SOI[2];   // 'B', 'M'
        uint32_t FSIZE;
        uint32_t RESERVED; // this is actually two consecutive 16 bit elements, but who cares :)
        uint32_t PIXELDATASTART;
} BITMAPFILEHEADER;
        #pragma pack(pop)

    // image header comes in two variants!
    // one representing OS/2 BMP format (BITMAPCOREHEADER) and another representing the most common Windows BMP format.
    // (BITMAPINFOHEADER)
    // however there are no markers to identify the type of the image header present in the bmp image.
    // the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct.
    // sizeof BITMAPCOREHEADER is 12 bytes
    // sizeof BITMAPINFOHEADER is >= 40 bytes.
    // from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes!

        #pragma pack(push, 1)
typedef struct {
        uint32_t        HEADERSIZE; // >= 40 bytes.
        uint32_t        WIDTH;      // width of the bitmap image in pixels
        int32_t         HEIGHT;     // height of the bitmap image in pixels
        // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
        // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
        uint16_t        NPLANES;       // must be 1
        uint16_t        NBITSPERPIXEL; // 1, 4, 8, 16, 24 or 32
        COMPRESSIONKIND CMPTYPE;       // compression kind
        uint32_t        IMAGESIZE;     // 0 if not compressed.
        uint32_t        RESPPMX;       // resolution in pixels per meter along x axis.
        uint32_t        RESPPMY;       // resolution in pixels per meter along y axis.
        uint32_t        NCMAPENTRIES;  // number of entries in the colourmap that are used.
        uint32_t        NIMPCOLORS;    // number of important colors.
} BITMAPINFOHEADER;
        #pragma pack(pop)

    // a BMP with BITMAPCOREHEADER cannot be compressed and is very rarely used in modern BMPs.

        #pragma pack(push, 1)
typedef struct {
        uint32_t HEADERSIZE; // 12 bytes
        uint16_t WIDTH;
        uint16_t HEIGHT;
        uint16_t NPLANES;       // must be 1
        uint16_t NBITSPERPIXEL; // could be 1, 4, 8 or 24
} BITMAPCOREHEADER;

// struct used to represent RGB pixels in BMP images (more widely uised compared to RGBTRIPLE)
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
        uint8_t RESERVED; // must be 0, but seems to be 0xFF in most BMPs.
} RGBQUAD;

// BMP files in OS/2 use this variant.
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
} RGBTRIPLE;
        #pragma endregion _HANDROLLED_STRUCTS_
    #endif // !__USE_HANDROLLED_BMP_STRUCTS__

// order of pixels in the BMP buffer.
typedef enum { TOPDOWN, BOTTOMUP } BMPPIXDATAORDERING;

// mechanism to be used in converting RGB pixels to black and white.
typedef enum { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY } TOBWKIND;

// combinations of RGBs to remove from pixels.
typedef enum { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE } RGBCOMB;

// types of compressions used in BMP files.
typedef enum { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN } COMPRESSIONKIND;

// a struct representing a BMP image
typedef struct bmp {
        BITMAPFILEHEADER fileheader;
        BITMAPINFOHEADER infoheader;
        RGBQUAD*         pixels; // this points to the start of pixels in the file buffer i.e (buffer + 54)
        uint8_t*         buffer; // this will point to the original file buffer, this is the one that needs deallocation!
} bmp_t;

static const uint16_t SOI = 0x4D42; // that's 'M' followed by a 'B' (LE)
// wingdi's BITMAPFILEHEADER uses a uint16_t for Start Of Image instead of two chars

static __forceinline BITMAPFILEHEADER __cdecl ParseFileHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
    // static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER must be 14 bytes in size");
    // above won't work with wingdi's structs as they aren't packed. So commenting out the above line:(
    assert(length >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER header = { .bfType = 0, .bfSize = 0, .bfReserved1 = 0, .bfReserved2 = 0, .bfOffBits = 0 };

    if (('B' != imstream[0]) && ('M' != imstream[1])) { // validate that the passed buffer is of a BMP file
        _putws(L"Error in ParseFileHeader, file isn't a Windows BMP file");
        return header;
    }

    header.bfType    = SOI;
    header.bfSize    = *(uint32_t*) (imstream + 2); // file size in bytes
    // 4 bytes skipped (bfReserved1, bfReserved2)
    header.bfOffBits = *(uint32_t*) (imstream + 10); // offset to the start of pixel data
    return header;
}

// find the type of compression used in the BMP file
// BMP files, in general aren't compressed
static __forceinline COMPRESSIONKIND __cdecl GetCompressionKind(_In_ const uint32_t compressionkind) {
    switch (compressionkind) {
        case 0  : return RGB;
        case 1  : return RLE8;
        case 2  : return RLE4;
        case 3  : return BITFIELDS;
        default : break;
    }
    return UNKNOWN;
}

static __forceinline BITMAPINFOHEADER __cdecl ParseInfoHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
    // alignment of wingdi's BITMAPINFOHEADER members makes it inherently packed :)
    static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "Error: BITMAPINFOHEADER must be 40 bytes in size");
    assert(length >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

    BITMAPINFOHEADER header = {
        .biSize          = 0,
        .biWidth         = 0,
        .biHeight        = 0,
        .biPlanes        = 0,
        .biBitCount      = 0,
        .biCompression   = 0,
        .biSizeImage     = 0,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed       = 0,
        .biClrImportant  = 0,
    };

    if (*((uint32_t*) (imstream + 14U)) > 40) {
        fputws(L"Error in ParseInfoHeader: Unparseable BITMAPINFOHEADER", stderr);
        return header;
    }

    header.biSize          = *(uint32_t*) (imstream + 14);
    header.biWidth         = *(int32_t*) (imstream + 18);
    header.biHeight        = *(int32_t*) (imstream + 22);
    header.biPlanes        = *(uint16_t*) (imstream + 26);
    header.biBitCount      = *(uint16_t*) (imstream + 28);
    header.biCompression   = GetCompressionKind(*(uint32_t*) (imstream + 30U));
    header.biSizeImage     = *(uint32_t*) (imstream + 34);
    header.biXPelsPerMeter = *(int32_t*) (imstream + 38);
    header.biYPelsPerMeter = *(int32_t*) (imstream + 42);
    header.biClrUsed       = *(uint32_t*) (imstream + 46);
    header.biClrImportant  = *(uint32_t*) (imstream + 50);

    return header;
}

static __forceinline BMPPIXDATAORDERING __cdecl GetPixelOrder(_In_ const BITMAPINFOHEADER header) {
    return (header.biHeight >= 0) ? BOTTOMUP : TOPDOWN;
}

static inline bool __cdecl BmpWrite(
    _In_ const wchar_t* const restrict filepath, _In_ bmp_t* const restrict image, _In_ const bool cleanup
) {
    // since we are using wingdi's struct definitions, our hands are pretty much tied when it comes to control over the memory layout of the structs
    // WE CANNOT ASSUME THAT THE STRUCTS WILL BE PACKED.

    // ther are two options to handle the two buffers
    // we could use two Win32 IO calls, using CreateFileW, first one with a CREATE_ALWAYS flag to serialize the structs and the second one with
    // OPEN_EXISTING | FILE_APPEND_DATA flags to append the pixel buffer to the existing file
    // this will entail a performance overhead of a separate system call.

    // or we could use a separate allocation, large enough to accomodate the structs and the pixel buffer and then
    // we could use this buffer to serialize the structs and pixels into and then write everything to disk at once.
    // this will eliminate the overhead of using two IO calls while brining in the penalty of a separate heap allocation.
    // TRADEOFFS :(

    static HANDLE64 hProcHeap = NULL;

    if (!Serialize(filepath, image->buffer, image->fileheader.bfSize, false)) {
        //  Serialize will take care of the error reposting, caller doesn't have to.
        fputws(L"Call to Serialize() inside BmpWrite failed!", stderr);
        return false;
    }

    if (cleanup) {
        hProcHeap = GetProcessHeap();
        if (hProcHeap == INVALID_HANDLE_VALUE) {
            fputws(L"GetProcessHeap() returned INVALID_HANDLE_VALUE inside BmpWrite!", stderr);
            return false;
        }

        if (!HeapFree(hProcHeap, 0, image->buffer)) {
            fputws(L"Impending memory leak! HeapFree() failed inside BmpWrite!", stderr);
            return false;
        }

        image->buffer = image->pixels = NULL;
    }

    return true;
}

static inline bmp_t __cdecl BmpRead(_In_ const wchar_t* const restrict filepath) {
    size_t size                 = 0;
    bmp_t  image                = { 0 }; // will be used as an empty placeholder for premature returns until members are properly assigned

    const uint8_t* const buffer = Open(filepath, &size); // HeapFree()
    if (!buffer) return image;                           // Open will do the error reporting, so just exiting the function is enough

    const BITMAPFILEHEADER fhead = ParseFileHeader(buffer, size); // 14 bytes (packed)
    if (!fhead.bfSize) return image; // again ParseFileHeader will report errors, if the predicate isn't satisified, exit the routine

    const BITMAPINFOHEADER infhead = ParseInfoHeader(buffer, size); // 40 bytes (packed)
    if (!infhead.biSize) return image;                              // error reporting is handled by ParseInfoHeader

    // creating and using a new buffer to only store pixels sounds like a clean idea but it brings a string of performance issues
    // 1) an additional heap allocation for the new buffer and deallocation of the original buffer
    // 2) now that the buffer only holds pixels, we'll need to serialize the structs separately when serializing the image
    // either constructing a temporary array of 54 bytes that hold the structs, and calling CreateFileW with CREATE_NEW first to serialize the
    // structs, closing that file and then reopening it using CreateFileW with OPEN_EXISTING and FILE_APPEND_DATA to append the pixel buffer
    // paying the penalty for two IO calls.
    // 3) or we could create a new buffer with enough space for the structs and pixels and then copy the structs and pixels there first,
    // followed by serialization of this new buffer with one call to CreateFileW, the caveat here is a gratuitous allocation and deallocation
    // const size_t npixels = (size - 54) / 4; // RGBQUAD consumes 4 bytes

    // const HANDLE64 hProcHeap = GetProcessHeap();
    // if (hProcHeap == INVALID_HANDLE_VALUE) {
    //     fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
    //     return image;
    // }

    // uint8_t*     pixels  = NULL;
    // if (!(pixels = HeapAlloc(hProcHeap, 0, size - 54))) { }
    // even though bmp_t's `pixels` member is declared as an array of RGBQUADs, we will not be creating RGBQUADs before writing them to the buffer.
    // the compiler may choose to optimize this away but performance wise this is too hefty a price to pay.
    // copying the raw bytes will make no difference granted that we dereference the pixels buffer at appropriate 4 byte intervals as RGBQUADs.

    // if stuff goes left, memcpy_s will raise an access violation exception, not bothering error handling here.
    // memcpy_s(pixels, size - 54, buffer + 54, size - 54);
    image.fileheader = fhead;
    image.infoheader = infhead;
    image.buffer     = buffer;
    image.pixels     = (RGBQUAD*) (buffer + 54);
    // HeapFree(hProcHeap, 0, buffer); // loose the raw bytes buffer

    return image;
}

// prints out information about the passed BMP file
static inline void __cdecl BmpInfo(_In_ const bmp_t* const image) {
    wprintf_s(
        L"|---------------------------------------------------------------------------|"
        L"%15s bitmap image (%3.4Lf MiBs)\n"
        L"Pixel ordering: %10s\n"
        L"Width: %5lu pixels, Height: %5lu pixels\n"
        L"Bit depth: %3u\n"
        L"Resolution (PPM): X {%5ld} Y {%5ld}\n"
        L"|---------------------------------------------------------------------------|",
        image->infoheader.biSizeImage ? L"Compressed" : L"Uncompressed",
        image->fileheader.bfSize / (1024.0L * 1024.0L),
        GetPixelOrder(image->infoheader) == BOTTOMUP ? L"bottom-up" : L"top-down",
        image->infoheader.biWidth,
        image->infoheader.biHeight,
        image->infoheader.biBitCount,
        image->infoheader.biXPelsPerMeter,
        image->infoheader.biYPelsPerMeter
    );

    if (image->infoheader.biSizeImage) { // don't bother if the image isn't compressed
        switch (image->infoheader.biCompression) {
            case RGB       : _putws(L"RGB"); break;
            case RLE4      : _putws(L"RLE4"); break;
            case RLE8      : _putws(L"RLE8"); break;
            case BITFIELDS : _putws(L"BITFIELDS"); break;
            case UNKNOWN   : _putws(L"UNKNOWN"); break;
            default        : break;
        }
    }
}

// if inplace = true, the returned struct could be safely ignored (it'll just be an empty bmp_t struct)
static inline bmp_t __cdecl ToBWhite(_In_ bmp_t* const image, _In_ const TOBWKIND conversionkind, _In_ const bool inplace) {
    HANDLE64    hProcHeap = NULL;
    uint8_t*    buffer    = NULL;
    RGBQUAD*    pixels    = NULL;
    const bmp_t temp      = { .fileheader = { 0 }, .infoheader = { 0 }, .buffer = NULL, .pixels = NULL };

    if (!inplace) { // if a new image is requested, allocate a new pixel buffer.
        hProcHeap = GetProcessHeap();
        if (hProcHeap == INVALID_HANDLE_VALUE) {
            fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
            return temp;
        }
        buffer = HeapAlloc(hProcHeap, 0, image->fileheader.bfSize);
        if (!buffer) {
            fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
            return temp;
        }
        // we still need to copy over the structs though, just the first 54 bytes.
        memcpy_s(buffer, 54, image->buffer, 54);
        pixels = (RGBQUAD*) (buffer + 54); // space for the structs
    } else
        pixels = image->pixels;            // if the image is to be modified inplace, copy its pixel buffer's address

    // no matter how the caller specifies the inplace argument, pixles will point to a valid memory now
    const size_t npixels = image->infoheader.biHeight * image->infoheader.biWidth;

    switch (conversionkind) {
        case AVERAGE : // just get the arithmetic average of all three RGB values
            for (size_t i = 0; i < npixels; ++i)
                pixels[i].rgbBlue = pixels[i].rgbGreen = pixels[i].rgbRed =
                    (((long double) (image->pixels[i].rgbBlue)) + image->pixels[i].rgbGreen + image->pixels[i].rgbRed) / 3.0L;
            break;

        case WEIGHTED_AVERAGE : // RGB values are averaged using predetermined weights
            for (size_t i = 0; i < npixels; ++i)
                pixels[i].rgbBlue = pixels[i].rgbGreen = pixels[i].rgbRed =
                    (image->pixels[i].rgbBlue * 0.299L + image->pixels[i].rgbGreen * 0.587L + image->pixels[i].rgbRed * 0.114L);
            break;

        case LUMINOSITY :
            for (size_t i = 0; i < npixels; ++i)
                pixels[i].rgbBlue = pixels[i].rgbGreen = pixels[i].rgbRed =
                    (image->pixels[i].rgbBlue * 0.2126L + image->pixels[i].rgbGreen * 0.7152L + image->pixels[i].rgbRed * 0.0722L);
            break;

        case BINARY :
            for (size_t i = 0; i < npixels; ++i)
                pixels[i].rgbBlue = pixels[i].rgbGreen = pixels[i].rgbRed =
                    (((int32_t) (image->pixels[i].rgbBlue)) + image->pixels[i].rgbGreen + image->pixels[i].rgbRed) / 3 >= 128 ? 255 : 0;
            break;
    }
    return inplace ? temp :
                     (bmp_t) { .fileheader = image->fileheader, .infoheader = image->infoheader, .buffer = buffer, .pixels = pixels };
}

static inline bmp_t __cdecl ToNegative(_In_ bmp_t* const image, _In_ const bool inplace) {
    HANDLE64    hProcHeap = NULL;
    uint8_t*    buffer    = NULL;
    RGBQUAD*    pixels    = NULL;
    const bmp_t temp      = { .fileheader = { 0 }, .infoheader = { 0 }, .buffer = NULL, .pixels = NULL };

    if (!inplace) { // if a new image is requested, allocate a new pixel buffer.
        hProcHeap = GetProcessHeap();
        if (hProcHeap == INVALID_HANDLE_VALUE) {
            fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
            return temp;
        }
        buffer = HeapAlloc(hProcHeap, 0, image->fileheader.bfSize);
        if (!buffer) {
            fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
            return temp;
        }
        // copy the metadata structs over to the new buffer.
        memcpy_s(buffer, 54, image->buffer, 54);
        pixels = (RGBQUAD*) (buffer + 54);
    } else
        pixels = image->pixels; // if the image is to be modified inplace, copy its pixel buffer's address

    for (int64_t i = 0; i < image->infoheader.biHeight * image->infoheader.biWidth /* number of total pixels */; ++i) {
        pixels[i].rgbBlue  = image->pixels[i].rgbBlue >= 128 ? 255 : 0;
        pixels[i].rgbGreen = image->pixels[i].rgbGreen >= 128 ? 255 : 0;
        pixels[i].rgbRed   = image->pixels[i].rgbRed >= 128 ? 255 : 0;
        // let the reserved byte be.
    }
    return inplace ? temp :
                     (bmp_t) { .fileheader = image->fileheader, .infoheader = image->infoheader, .buffer = buffer, .pixels = pixels };
}

// removes specified RGB components from the image pixels
static inline bmp_t __cdecl RemoveColour(_In_ bmp_t* const image, _In_ const RGBCOMB colourcombination, _In_ const bool inplace) {
    HANDLE64    hProcHeap = NULL;
    uint8_t*    buffer    = NULL;
    RGBQUAD*    pixels    = NULL;
    const bmp_t temp      = { .fileheader = { 0 }, .infoheader = { 0 }, .buffer = NULL, .pixels = NULL };

    if (!inplace) { // if a new image is requested, allocate a new pixel buffer.
        hProcHeap = GetProcessHeap();
        if (hProcHeap == INVALID_HANDLE_VALUE) {
            fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
            return temp;
        }
        buffer = HeapAlloc(hProcHeap, 0, image->fileheader.bfSize);
        if (!buffer) {
            fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
            return temp;
        }
        // we need to copy over the complete buffer
        memcpy_s(buffer, image->fileheader.bfSize, image->buffer, image->fileheader.bfSize);
        pixels = (RGBQUAD*) (buffer + 54);
    } else
        pixels = image->pixels; // if the image is to be modified inplace, copy its pixel buffer's address

    const int64_t npixels = image->infoheader.biHeight * image->infoheader.biWidth;
    switch (colourcombination) {
        case BLUE :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbBlue = 0;
            break;
        case GREEN :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbGreen = 0;
            break;
        case RED :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbRed = 0;
            break;
        case REDGREEN :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbRed = pixels[i].rgbGreen = 0;
            break;
        case REDBLUE :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbRed = pixels[i].rgbBlue = 0;
            break;
        case GREENBLUE :
            for (int64_t i = 0; i < npixels; ++i) pixels[i].rgbGreen = pixels[i].rgbBlue = 0;
            break;
    }
    return inplace ? temp :
                     (bmp_t) { .fileheader = image->fileheader, .infoheader = image->infoheader, .buffer = buffer, .pixels = pixels };
}

// TODO: Implementation works fine only when width and height are divisible by 256 without remainders. SORT THIS OUT!
static inline bmp_t __cdecl GenGradient(_In_ const long npixels_h, _In_ const long npixels_w) {
    bmp_t          image     = { .fileheader = { 0 }, .infoheader = { 0 }, .buffer = NULL, .pixels = NULL };
    uint8_t*       buffer    = NULL;
    RGBQUAD*       pixels    = NULL;
    const HANDLE64 hProcHeap = GetProcessHeap();

    if ((npixels_h % 256) || (npixels_w % 256)) {
        fwprintf_s(stderr, L"Dimensions need to be multiples of 256! Received (%5zu,%5zu)", npixels_h, npixels_w);
        return image;
    }

    if (hProcHeap == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
        return image;
    }
    buffer = HeapAlloc(hProcHeap, 0, (sizeof(RGBQUAD) * npixels_w * npixels_h) + 54); // add 54 bytes for the two structs
    if (!buffer) {
        fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
        return image;
    }

    const BITMAPFILEHEADER tmpfile = { .bfType = SOI,
                                       // packing assumed
                                       .bfSize =
                                           sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * npixels_h * npixels_w),
                                       .bfOffBits   = 54,
                                       .bfReserved1 = 0,
                                       .bfReserved2 = 0 };

    const BITMAPINFOHEADER tmpinfo = { .biSize          = 40,
                                       .biWidth         = npixels_w,
                                       .biHeight        = npixels_h,
                                       .biPlanes        = 1,
                                       .biBitCount      = 32, // three bytes per RGB pixel
                                       .biCompression   = RGB,
                                       .biSizeImage     = 0,
                                       .biXPelsPerMeter = 0,
                                       .biYPelsPerMeter = 0,
                                       .biClrUsed       = 0,
                                       .biClrImportant  = 0 };

    // jump through the pixel matrix, using a window frame
    // within the rows of a window frame, gradually increment the RED value
    // along columns of a window, gradually increment the GREEN value.
    // increment the BLUE pixel across subsequent frames.

    // the deal here is that we must keep at least one RGB component constant within these stride windows. (our pick is BLUE)
    const size_t hstride = npixels_w / 256, vstride = npixels_h / 256; // NOLINT(readability-isolate-declaration)
    uint8_t      red = 0xFF, green = 0xFF, blue = 0x00;                // NOLINT(readability-isolate-declaration)

    // for moving across pixel rows, by a select stride, captures row frames, stride by stride.
    for (size_t v = 0; v < npixels_h /* for all pixels in a column */
         ;
         v += vstride) {
        // for each pixel row within that vertical stride window (a row frame with vstride rows in it, one below another)
        for (size_t j = v; j < vstride; ++j) {
            // for moving across pixel columns, partitions rows horizontally into pixel frames with hstride columns in each of them
            for (size_t h = 0; h < npixels_w /* for all pixels in a row */; h += hstride) { // NOLINT(readability-identifier-length)
                red++;
                // for each pixel within the captured pixel frame with hstride columns and vstride rows
                for (size_t i = h; i < hstride; ++i) {
                    pixels[(j * npixels_w) + i].rgbBlue     = blue;
                    pixels[(j * npixels_w) + i].rgbGreen    = green;
                    pixels[(j * npixels_w) + i].rgbRed      = red;
                    pixels[(j * npixels_w) + i].rgbReserved = 0xFF;
                }
            }
            green--;
        }
        blue++;
    }
    return (bmp_t) { .fileheader = tmpfile, .infoheader = tmpinfo, .buffer = buffer, .pixels = pixels };
}

// a struct to specify the factors to scale the colours by
// minimum scale factor is 0.0, values less than 0.0 (negatives) will be considered 0.0
// maximum scale factor is 1.0, values greater than 1.0 will be considered as 1.0
typedef struct cscale {
        float csBlue;
        float csGreen;
        float csRed;
} cscale_t;

// TODO incomplete implementation
static inline bmp_t __cdecl ScaleColors(_In_ bmp_t* const image, _In_ const cscale_t scale, _In_ const bool inplace) {
    HANDLE64 hProcHeap = NULL;
    RGBQUAD* pixels    = NULL;
    uint8_t* buffer    = NULL;
    bmp_t    temp      = { .fileheader = { 0 }, .infoheader = { 0 }, .buffer = NULL, .pixels = NULL };

    if (!inplace) { // if a new image is requested, allocate a new pixel buffer.
        hProcHeap = GetProcessHeap();
        if (hProcHeap == INVALID_HANDLE_VALUE) {
            fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
            return temp;
        }
        buffer = HeapAlloc(hProcHeap, 0, image->fileheader.bfSize - 54); // discount the 54 bytes occupied by the two structs
        if (!buffer) {
            fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
            return temp;
        }
    } else
        pixels = image->pixels; // if the image is to be modified inplace, copy its pixel buffer's address
}

#endif // !__BMP_H__
