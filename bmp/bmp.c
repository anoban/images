#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// clang-format off
#include <bmp.h>
#include <imageio.h>
#include <handleapi.h>
#include <heapapi.h>
#include <errhandlingapi.h>
// clang-format on

// ALL INLINE FUNCTIONS ARE MEANT TO BE USED WITHIN THIS TRANSLATION UNIT AND WON'T EXPOSE THEIR SIGNATURES IN <bmp.h>

static const uint16_t                 SOI = 0x4D42; // that's 'M' followed by a 'B' (LE)
// wingdi's BITMAPFILEHEADER uses a uint16_t for Start Of Image instead of two chars

static __forceinline BITMAPFILEHEADER ParseFileHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
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
static __forceinline COMPRESSIONKIND GetCompressionKind(_In_ const uint32_t compressionkind) {
    switch (compressionkind) {
        case 0 : return RGB;
        case 1 : return RLE8;
        case 2 : return RLE4;
        case 3 : return BITFIELDS;
    }
    return UNKNOWN;
}

static __forceinline BITMAPINFOHEADER ParseInfoHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
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
    header.biWidth         = *(uint32_t*) (imstream + 18);
    header.biHeight        = *(int32_t*) (imstream + 22);
    header.biPlanes        = *(uint16_t*) (imstream + 26);
    header.biBitCount      = *(uint16_t*) (imstream + 28);
    header.biCompression   = GetCompressionKind(*(uint32_t*) (imstream + 30U));
    header.biSizeImage     = *(uint32_t*) (imstream + 34);
    header.biXPelsPerMeter = *(uint32_t*) (imstream + 38);
    header.biYPelsPerMeter = *(uint32_t*) (imstream + 42);
    header.biClrUsed       = *(uint32_t*) (imstream + 46);
    header.biClrImportant  = *(uint32_t*) (imstream + 50);

    return header;
}

static __forceinline BMPPIXDATAORDERING GetPixelOrder(_In_ const BITMAPINFOHEADER header) {
    return (header.biHeight >= 0) ? BOTTOMUP : TOPDOWN;
}

bool BmpWrite(_In_ const wchar_t* const restrict filepath, _In_ const bmp_t* const restrict image) {
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
}

bmp_t BmpRead(_In_ const wchar_t* const restrict filepath) {
    size_t               size   = 0;
    bmp_t                image  = { 0 }; // will be used as an empty placeholder for premature returns until members are properly assigned
    const uint8_t* const buffer = Open(filepath, &size); // HeapFree()
    if (!buffer) return image;                           // Open will do the error reporting, so just exiting the function is enough
    const BITMAPFILEHEADER fhead = ParseFileHeader(buffer, size); // 14 bytes (packed)
    if (!fhead.bfSize) return image; // again ParseFileHeader will report errors, if the predicate isn't satisified, exit the routine
    const BITMAPINFOHEADER infhead = ParseInfoHeader(buffer, size); // 40 bytes (packed)
    if (!infhead.biSize) return image;                              // error reporting is handled by ParseInfoHeader

    const size_t   npixels   = (size - 54) / 4; // RGBQUAD consumes 4 bytes
    const HANDLE64 hProcHeap = GetProcessHeap();
    if (hProcHeap == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
        return image;
    }

    uint8_t* pixels = NULL;
    if (!(pixels = HeapAlloc(hProcHeap, 0, size - 54))) { }
    // even though bmp_t's `pixels` member is declared as an array of RGBQUADs, we will not be creating RGBQUADs before writing them to the buffer.
    // the compiler may choose to optimize this away but performance wise this is too hefty a price to pay.
    // copying the raw bytes will make no difference granted that we dereference the pixels buffer at appropriate 4 byte intervals as RGBQUADs.

    // if stuff goes left, memcpy_s will raise an access violation exception, not bothering error handling here.
    memcpy_s(pixels, size - 54, buffer + 54, size - 54);
    image.fileheader = fhead;
    image.infoheader = infhead;
    image.pixels     = (RGBQUAD*) pixels;
    HeapFree(hProcHeap, 0, buffer); // loose the raw bytes buffer

    return image;
}

// prints out information about the passed BMP file
void BmpInfo(_In_ const bmp_t* const image) {
    wprintf_s(
        L"|---------------------------------------------------------------------------|"
        L"%15s bitmap image (%3.4Lf MiBs)\n"
        L"Pixel ordering: %10s\n"
        L"Width: %5u pixels, Height: %5u pixels\n"
        L"Bit depth: %3u\n"
        L"Resolution (PPM): X {%5ld} Y {%5ld}\n"
        L"|---------------------------------------------------------------------------|",
        image->fileheader.bfSize / (1024.0L * 1024.0L),
        image->infoheader.biSizeImage ? L"Compressed" : L"Uncompressed",
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
        }
    }
    return;
}

// if inplace = true, the returned struct could be safely ignored (it'll be a skeletal copy of the struct passed into the routine, with the same buffer)
bmp_t ToBWhite(_In_ bmp_t* const image, _In_ const TOBWKIND conversionkind, _In_ const bool inplace) {
    HANDLE64 hProcHeap = NULL;
    RGBQUAD* altbuffer = NULL;
    if (!inplace) {
        hProcHeap = GetProcessHeap();
        altbuffer = HeapAlloc(hProcHeap, 0, image->fileheader.bfSize - 54); // discount the 54 bytes occupied by the two structs
    }

    bmp_t result = *image;
    switch (conversionkind) {
        case AVERAGE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = <uint8_t>((<long double>(pix.BLUE) + pix.GREEN + pix.RED) / 3);
            });
            break;

        case WEIGHTED_AVERAGE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = <uint8_t>((pix.BLUE * 0.299L) + (pix.GREEN * 0.587) + (pix.RED * 0.114));
            });
            break;

        case LUMINOSITY :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = <uint8_t>((pix.BLUE * 0.2126L) + (pix.GREEN * 0.7152L) + (pix.RED * 0.0722L));
            });
            break;

        case BINARY :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = (<uint64_t>(pix.BLUE) + pix.GREEN + pix.RED) / 3 >= 128 ? 255Ui8 : 0Ui8;
            });
            break;
    }
}

bmp_t ToNegative(_In_ bmp_t* const image, _In_ const bool inplace) {
    if (inplace) {
        for (size_t i = 0; i < image->infoheader.biHeight * image->infoheader.biWidth /* number of total pixels */; ++i) {
            image->pixels[i].rgbBlue  = image->pixels[i].rgbBlue >= 128 ? 255 : 0;
            image->pixels[i].rgbGreen = image->pixels[i].rgbGreen >= 128 ? 255 : 0;
            image->pixels[i].rgbRed   = image->pixels[i].rgbRed >= 128 ? 255 : 0;
            // let the reserved byte be.
        }
        return *image;
    }
}

bmp_t RemoveColour(_In_ bmp_t* const image, _In_ const RGBCOMB colourcombination, _In_ const bool inplace) {
    switch (colourcombination) {
        case BLUE      : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.BLUE = 0; }); break;
        case GREEN     : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = 0; }); break;
        case RED       : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = 0; }); break;
        case REDGREEN  : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.GREEN = 0; }); break;
        case REDBLUE   : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.BLUE = 0; }); break;
        case GREENBLUE : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = pix.BLUE = 0; }); break;
    }
}

// TODO: Implementation works fine only when width and height are divisible by 256 without remainders. SORT THIS OUT!
// DO NOT repeat the static keyword here! It's enough to declare the method as static only in the header file.
bmp_t GenGradient(_In_ const size_t heightpx, _In_ const size_t widthpx) {
    bmp_t image = { 0 };
    if (((heightpx % 256) != 0) || ((widthpx % 256) != 0))
        fwprintf_s(stderr, L"Dimensions need to be multiples of 256! Received ({}, {})", heightpx, widthpx);

    BITMAPFILEHEADER tmpfile = { .bfType    = SOI,
                                 // packing assumed
                                 .bfSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * heightpx * widthpx),
                                 .bfOffBits = 54,
                                 .bfReserved1 = 0,
                                 .bfReserved2 = 0 };

    BITMAPINFOHEADER tmpinfo = { .HEADERSIZE    = 40,
                                 .WIDTH         = <uint32_t>(widthpx),
                                 .HEIGHT        = <int32_t>(heightpx),
                                 .NPLANES       = 1,
                                 .NBITSPERPIXEL = 32,
                                 .CMPTYPE       = RGB,
                                 .IMAGESIZE     = 0,
                                 .RESPPMX       = 0,
                                 .RESPPMY       = 0,
                                 .NCMAPENTRIES  = 0,
                                 .NIMPCOLORS    = 0 };

    vector<RGBQUAD>  pixels {};
    pixels.reserve(widthpx * heightpx);

    // the idea to create a colour gradient =>
    // traverse through the pixel buffer, within a row gradually increment the RED value
    // within a column, gradually increment the GREEN value.

    // the deal here is that we must keep at least one RGB component constant within windows of this width.
    const size_t hstride { widthpx / 256 }, vstride { heightpx / 256 };

    uint8_t      R { 0xFF }, G { 0xFF }, B { 0x00 };

    for (size_t h = 0; h < heightpx; h += vstride) {
        for (size_t x = 0; x < vstride; ++x) {
            for (size_t w = 0; w < widthpx; w += hstride) {
                R++;
                for (size_t i = 0; i < hstride; ++i) pixels.push_back(RGBQUAD { .BLUE = B, .GREEN = G, .RED = R, .RESERVED = 0xFF });
                G--;
            }
        }
        B++;
    }
    return (bmp_t) { file, info, pixels };
}