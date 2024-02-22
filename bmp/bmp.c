#include <assert.h>
#include <bmp.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <sal.h>
#include <stdio.h>

static const uint16_t SOI = 0x4D42; // that's 'M' followed by a 'B' (LE)
// WinGdi's BITMAPFILEHEADER uses a uint16_t for Start Of Image instead of two chars

static __forceinline BITMAPFILEHEADER __stdcall ParseFileHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
    // static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER must be 14 bytes in size");
    // above won't work with WinGdi's structs as they aren't packed. So commenting out the above line:(
    assert(length >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER header = { .bfType = 0, .bfSize = 0, .bfReserved1 = 0, .bfReserved2 = 0, .bfOffBits = 0 };

    if (('B' != imstream[0]) && ('M' != imstream[1])) {
        _putws(L"Error in ParseFileHeader, file isn't a Windows BMP file\n");
        return header;
    }

    header.bfType    = SOI;
    header.bfSize    = *(uint32_t*) (imstream + 2);  // file size in bytes
    header.bfOffBits = *(uint32_t*) (imstream + 10); // offset to the start of pixel data
    return header;
}

static __forceinline COMPRESSIONKIND __stdcall GetCompressionKind(_In_ const uint32_t cmpkind) {
    switch (cmpkind) {
        case 0 : return RGB;
        case 1 : return RLE8;
        case 2 : return RLE4;
        case 3 : return BITFIELDS;
    }
    return UNKNOWN;
}

static __forceinline BITMAPINFOHEADER __stdcall ParseInfoHeader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
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

    header.biSize          = *(const uint32_t*) (imstream + 14);
    header.biWidth         = *(const uint32_t*) (imstream + 18);
    header.biHeight        = *(const int32_t*) (imstream + 22);
    header.biPlanes        = *(const uint16_t*) (imstream + 26);
    header.biBitCount      = *(const uint16_t*) (imstream + 28);
    header.biCompression   = GetCompressionKind(*(const uint32_t*) (imstream + 30U));
    header.biSizeImage     = *(const uint32_t*) (imstream + 34);
    header.biXPelsPerMeter = *(const uint32_t*) (imstream + 38);
    header.biYPelsPerMeter = *(const uint32_t*) (imstream + 42);
    header.biClrUsed       = *(const uint32_t*) (imstream + 46);
    header.biClrImportant  = *(const uint32_t*) (imstream + 50);

    return header;
}

static __forceinline BMPPIXDATAORDERING __stdcall get_pixelorder(_In_ const BITMAPINFOHEADER header) {
    return (header.biHeight >= 0) ? BOTTOMUP : TOPDOWN;
}

void  BmpWrite(_In_ const wchar_t* const restrict filepath, _In_ const bmp_t* const restrict image) { }

bmp_t BmpRead(_In_ const wstring& path) {
    auto fbuff { open(path, &size) };
    fh      = ParseFileHeader(fbuff);
    infh    = ParseInfoHeader(fbuff);

    npixels = (size - 54) / 4;
    for (auto it = fbuff.begin() + 54; it != fbuff.cend(); it += 4) pixels.push_back(RGBQUAD { *it, *(it + 1), *(it + 2), *(it + 3) });
}

void BmpInfo(_In_ const bmp_t* const image) {
    wprintf_s(
        L"File size %Lf MiBs\nPixel data start offset: %d\n"
        L"BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        L"Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries: % u\n"
        L"Number of important colors: % u\n",
        ((long double) (fh.FSIZE) / (1024 * 1024LLU)),
        image->.PIXELDATASTART,
        image->infoheader.HEADERSIZE,
        image->infoheader.WIDTH,
        image->infoheader.HEIGHT,
        image->infoheader.NPLANES,
        image->infoheader.NBITSPERPIXEL,
        image->infoheader.IMAGESIZE,
        image->infoheader.RESPPMX,
        image->infoheader.RESPPMY,
        image->infoheader.NCMAPENTRIES,
        image->infoheader.NIMPCOLORS
    );

    switch (image->infoheader.biCompression) {
        case RGB       : _putws(L"BITMAPINFOHEADER.CMPTYPE: RGB"); break;
        case RLE4      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE4"); break;
        case RLE8      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE8"); break;
        case BITFIELDS : _putws(L"BITMAPINFOHEADER.CMPTYPE: BITFIELDS"); break;
        case UNKNOWN   : _putws(L"BITMAPINFOHEADER.CMPTYPE: UNKNOWN"); break;
    }

    wprintf_s(
        L"%s BMP file\n"
        L"BMP pixel ordering: %s\n",
        infh.IMAGESIZE != 0 ? L"Compressed" : L"Uncompressed",
        get_pixelorder(infh) == BOTTOMUP ? L"BOTTOMUP" : L"TOPDOWN"
    );

    return;
}

tobwhite(_In_ const TOBWKIND cnvrsnkind, _In_opt_ const bool inplace) {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace)
        imptr = this;
    else {
        [[likely]] copy = *this; // copy the bmp instance
        imptr           = &copy;
    }

    switch (cnvrsnkind) {
        case AVERAGE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((static_cast<long double>(pix.BLUE) + pix.GREEN + pix.RED) / 3);
            });
            break;

        case WEIGHTED_AVERAGE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.299L) + (pix.GREEN * 0.587) + (pix.RED * 0.114));
            });
            break;

        case LUMINOSITY :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.2126L) + (pix.GREEN * 0.7152L) + (pix.RED * 0.0722L));
            });
            break;

        case BINARY :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = (static_cast<uint64_t>(pix.BLUE) + pix.GREEN + pix.RED) / 3 >= 128 ? 255Ui8 : 0Ui8;
            });
            break;
    }
}

tonegative(_In_opt_ const bool inplace) {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
        pix.BLUE  = pix.BLUE >= 128 ? 255Ui8 : 0Ui8;
        pix.GREEN = pix.GREEN >= 128 ? 255Ui8 : 0Ui8;
        pix.RED   = pix.RED >= 128 ? 255Ui8 : 0Ui8;
    });

    if (inplace)
        return nullopt;
    else [[likely]]
        return copy;
}

optional<bmpbmp> remove_clr(_In_ const RGBCOMB kind, _In_opt_ const bool inplace) {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    switch (kind) {
        case RGBCOMBBLUE  : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.BLUE = 0; }); break;
        case RGBCOMBGREEN : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = 0; }); break;
        case RGBCOMBRED   : for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = 0; }); break;
        case RGBCOMBREDGREEN :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.GREEN = 0; });
            break;
        case RGBCOMBREDBLUE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.BLUE = 0; });
            break;
        case RGBCOMBGREENBLUE :
            for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = pix.BLUE = 0; });
            break;
    }
    if (inplace)
        return nullopt;
    else
        return copy;
}

// TODO: Implementation works fine only when width and height are divisible by 256 without remainders. SORT THIS OUT!
// DO NOT repeat the static keyword here! It's enough to declare the method as static only in the header file.
bmpbmp gengradient(_In_opt_ const size_t heightpx, _In_opt_ const size_t widthpx) {
    if (((heightpx % 256) != 0) || ((widthpx % 256) != 0))
        throw runtime_error(format("Dimensions need to be multiples of 256! Received ({}, {})", heightpx, widthpx));

    BITMAPFILEHEADER file {
        .SOI { array<uint8_t, 2> { { 'B', 'M' } } },
        .FSIZE          = static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * heightpx * widthpx),
        .PIXELDATASTART = 54
    };

    BITMAPINFOHEADER info { .HEADERSIZE    = 40,
                            .WIDTH         = static_cast<uint32_t>(widthpx),
                            .HEIGHT        = static_cast<int32_t>(heightpx),
                            .NPLANES       = 1,
                            .NBITSPERPIXEL = 32,
                            .CMPTYPE       = COMPRESSIONKINDRGB,
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
    return bmp(file, info, pixels);
}