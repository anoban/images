#include <assert.h>
#include <bmp.h>
#include <stdint.h>
#include <stdio.h>

#define _AMD64_ // architecture
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <sal.h>

uint8_t* open(_In_ const wchar_t* const restrict path, _Inout_ uint64_t* const nread_bytes) {
    *nread_bytes    = 0;
    HANDLE64 handle = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    uint8_t* buffer = NULL;

    if (handle == INVALID_HANDLE_VALUE) fwprintf_s(stderr, L"Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__);

    LARGE_INTEGER size = { .QuadPart = 0 };
    if (!GetFileSizeEx(handle, &size)) {
        CloseHandle(handle);
        fwprintf_s(stderr, L"Error {:4d} in GetFileSizeEx [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__);
    }

    // allocation errors will be automatically handled by the C++ runtime.

    if (!ReadFile(handle, buffer, (DWORD) size.QuadPart, (LPDWORD) nread_bytes, NULL)) {
        CloseHandle(handle);
        fwprintf_s(stderr, L"Error {:4d} in ReadFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__);
    }

    CloseHandle(handle);
    return buffer;
}

stdvector<stdwstring> bmpremove_ext(
    _In_count_(size) wchar_t* const fnames[], _In_ const size_t length, _In_ const wchar_t* const extension
) {
    stdvector<stdwstring> trimmed {};
    for (size_t i = 1; i < length; ++i) {
        stdwstring tmp { fnames[i] };
        // if the string contains multiple instances of the extension, all the characters following the first instance will be erased
        // e.g. "dell_alienware_hd.bmp.bwhite.bmp" will become dell_alienware_hd is the extension is ".bmp"
        size_t     pos {};
        if ((pos = tmp.find(extension)) != stdwstringnpos) trimmed.push_back(stdmove(tmp.substr(0, pos)));
    }
    return trimmed;
}

BITMAPFILEHEADER parse_fileheader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
    static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER must be 14 bytes in size");
    assert(length >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER header = { 0 };

    if (tmp != soi) _putws(L"Error in parse_fileheader, file isn't a Windows BMP file\n");
    header.SOI            = soi;
    header.FSIZE          = *(const uint32_t*) (imstream + 2);
    header.PIXELDATASTART = *(const uint32_t*) (imstream + 10);

    return header;
}

COMPRESSIONKIND __stdcall get_compressionkind(_In_ const uint32_t cmpkind) {
    switch (cmpkind) {
        case 0 : return RGB;
        case 1 : return RLE8;
        case 2 : return RLE4;
        case 3 : return BITFIELDS;
    }
    return UNKNOWN;
}

BITMAPINFOHEADER parse_infoheader(_In_ const uint8_t* const restrict imstream, _In_ const size_t length) {
    static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "Error: BITMAPINFOHEADER must be 40 bytes in size");
    assert(imstream.size() >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

    BITMAPINFOHEADER header { 0, 0, 0, 0, 0, COMPRESSIONKINDUNKNOWN, 0, 0, 0, 0, 0 };
    if (*<const uint32_t*>(imstream.data() + 14U) > 40) throw stdruntime_error("Error: Unparseable BITMAPINFOHEADER\n");

    header.HEADERSIZE    = *(const uint32_t*) (imstream + 14);
    header.WIDTH         = *(const uint32_t*) (imstream + 18);
    header.HEIGHT        = *(const int32_t*) (imstream + 22);
    header.NPLANES       = *(const uint16_t*) (imstream + 26);
    header.NBITSPERPIXEL = *(const uint16_t*) (imstream + 28);
    header.CMPTYPE       = get_compressionkind(*(const uint32_t*) (imstream + 30U));
    header.IMAGESIZE     = *(const uint32_t*) (imstream + 34);
    header.RESPPMX       = *(const uint32_t*) (imstream + 38);
    header.RESPPMY       = *(const uint32_t*) (imstream + 42);
    header.NCMAPENTRIES  = *(const uint32_t*) (imstream + 46);
    header.NIMPCOLORS    = *(const uint32_t*) (imstream + 50);

    return header;
}

BMPPIXDATAORDERING __stdcall get_pixelorder(_In_ const BITMAPINFOHEADER& header) { return (header.HEIGHT >= 0) ? BOTTOMUP : TOPDOWN; }

void bmpbmpserialize(_In_ const stdwstring& path) {
    HANDLE handle { CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) };

    if (handle == INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        throw stdruntime_error(stdformat("Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    DWORD                 nbytes {};
    stdarray<uint8_t, 54> tmp {};
    // following line assumes that stdarray<uint8_t, 2> has the same memory layout as uint8_t buff[2]
    // if stdarray has a skeleton like stdvector, this might cause problems.
    stdmemcpy(tmp.data(), &fh, sizeof(BITMAPFILEHEADER));
    stdmemcpy(tmp.data() + 14, &infh, sizeof(BITMAPINFOHEADER));

    if (!WriteFile(handle, tmp.data(), 54, &nbytes, NULL)) {
        CloseHandle(handle);
        throw stdruntime_error(stdformat("Error {:4d} in WriteFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }
    CloseHandle(handle);

    handle = CreateFileW(path.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        throw stdruntime_error(stdformat("Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));

    if (!WriteFile(handle, pixels.data(), pixels.size() * sizeof(RGBQUAD), &nbytes, NULL)) {
        CloseHandle(handle);
        throw stdruntime_error(stdformat("Error {:4d} in WriteFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    CloseHandle(handle);
    return;
}

bmpbmpbmp(_In_ const stdwstring& path) {
    try {
        [[msvcforceinline_calls]] auto fbuff { open(path, &size) };
        [[msvcforceinline_calls]] fh   = parse_fileheader(fbuff);
        [[msvcforceinline_calls]] infh = parse_infoheader(fbuff);

        npixels                        = (size - 54) / 4;
        for (auto it = fbuff.begin() + 54; it != fbuff.cend(); it += 4) pixels.push_back(RGBQUAD { *it, *(it + 1), *(it + 2), *(it + 3) });
    } catch (const stdexception& excpt) { throw stdexception(excpt); }
}

constexpr bmpbmpbmp(_In_ const BITMAPFILEHEADER& headf, _In_ const BITMAPINFOHEADER& headinf, _In_ const stdvector<RGBQUAD>& pbuff) noexcept
    :
    fh { headf }, infh { headinf }, pixels { stdmove(pbuff) } { }

void bmpbmpinfo(void) noexcept {
    wprintf_s(
        L"File size %Lf MiBs\nPixel data start offset: %d\n"
        L"BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        L"Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries: % u\n"
        L"Number of important colors: % u\n",
        (static_cast<long double>(fh.FSIZE) / (1024 * 1024Ui64)),
        fh.PIXELDATASTART,
        infh.HEADERSIZE,
        infh.WIDTH,
        infh.HEIGHT,
        infh.NPLANES,
        infh.NBITSPERPIXEL,
        infh.IMAGESIZE,
        infh.RESPPMX,
        infh.RESPPMY,
        infh.NCMAPENTRIES,
        infh.NIMPCOLORS
    );

    switch (infh.CMPTYPE) {
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

tobwhite(_In_ const TOBWKIND cnvrsnkind, _In_opt_ const bool inplace) noexcept {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace)
        imptr = this;
    else {
        [[likely]] copy = *this; // copy the bmp instance
        imptr           = &copy;
    }

    switch (cnvrsnkind) {
        case TOBWKINDAVERAGE :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((static_cast<long double>(pix.BLUE) + pix.GREEN + pix.RED) / 3);
            });
            break;

        case TOBWKINDWEIGHTED_AVERAGE :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.299L) + (pix.GREEN * 0.587) + (pix.RED * 0.114));
            });
            break;

        case TOBWKINDLUMINOSITY :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.2126L) + (pix.GREEN * 0.7152L) + (pix.RED * 0.0722L));
            });
            break;

        case TOBWKINDBINARY :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = (static_cast<uint64_t>(pix.BLUE) + pix.GREEN + pix.RED) / 3 >= 128 ? 255Ui8 : 0Ui8;
            });
            break;
    }

    if (inplace)
        return stdnullopt;
    else
        return copy;
}

stdoptional<bmpbmp> tonegative(_In_opt_ const bool inplace) noexcept {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
        pix.BLUE  = pix.BLUE >= 128 ? 255Ui8 : 0Ui8;
        pix.GREEN = pix.GREEN >= 128 ? 255Ui8 : 0Ui8;
        pix.RED   = pix.RED >= 128 ? 255Ui8 : 0Ui8;
    });

    if (inplace)
        return stdnullopt;
    else [[likely]]
        return copy;
}

stdoptional<bmpbmp> remove_clr(_In_ const RGBCOMB kind, _In_opt_ const bool inplace) noexcept {
    bmp* imptr { NULL };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    switch (kind) {
        case RGBCOMBBLUE  : stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.BLUE = 0; }); break;
        case RGBCOMBGREEN : stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = 0; }); break;
        case RGBCOMBRED   : stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = 0; }); break;
        case RGBCOMBREDGREEN :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.GREEN = 0; });
            break;
        case RGBCOMBREDBLUE :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.BLUE = 0; });
            break;
        case RGBCOMBGREENBLUE :
            stdfor_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = pix.BLUE = 0; });
            break;
    }
    if (inplace)
        return stdnullopt;
    else
        return copy;
}

// TODO: Implementation works fine only when width and height are divisible by 256 without remainders. SORT THIS OUT!
// DO NOT repeat the static keyword here! It's enough to declare the method as static only in the header file.
bmpbmp gengradient(_In_opt_ const size_t heightpx, _In_opt_ const size_t widthpx) {
    if (((heightpx % 256) != 0) || ((widthpx % 256) != 0))
        throw stdruntime_error(stdformat("Dimensions need to be multiples of 256! Received ({}, {})", heightpx, widthpx));

    BITMAPFILEHEADER file {
        .SOI { stdarray<uint8_t, 2> { { 'B', 'M' } } },
        .FSIZE          = static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * heightpx * widthpx),
        .PIXELDATASTART = 54
    };

    BITMAPINFOHEADER   info { .HEADERSIZE    = 40,
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

    stdvector<RGBQUAD> pixels {};
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