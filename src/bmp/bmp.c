#pragma warning(disable : 4710)

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #define _AMD64_ // architecture
    #define WIN32_LEAN_AND_MEAN
    #define WIN32_EXTRA_MEAN
#endif

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <sal.h>

// user defined datatypes that conflict with wingdi.h data type are prefixed with a double underscore to avoid conflicts.

static inline uint8_t* open_image(_In_ const wchar_t* restrict file_name, _Out_ uint64_t* const nread_bytes) {
    *nread_bytes    = 0;
    void *   handle = NULL, *buffer = NULL;
    uint32_t nbytes = 0;

    handle          = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (!GetFileSizeEx(handle, &file_size)) {
            fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", GetLastError());
            return NULL;
        }

        // add an extra megabyte to the buffer, for safety.
        size_t buffsize = file_size.QuadPart + (1024U * 1024);

        // caller is responsible for freeing this buffer.
        buffer          = malloc(buffsize);
        if (buffer) {
            if (ReadFile(handle, buffer, buffsize, &nbytes, NULL)) {
                *nread_bytes = nbytes;
                return buffer;
            } else {
                fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
                CloseHandle(handle);
                free(buffer);
                return NULL;
            }
        } else {
            fputws(L"Memory allocation error: malloc returned NULL", stderr);
            CloseHandle(handle);
            return NULL;
        }
    } else {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return NULL;
    }
}

// Almost every functionality related to BMPs seems to be abailable in wingdi.h yikes!
// Reinventing the wheel with 0 regrets :)

// .bmp is one of the simplest image encoding formats!
// so it'd help a lot to start with simple decoders and then move on to more complex ones.

// Windows .bmp images can be compressed, but is only valuable when the image has large blocks with identical colours.
// that severely restrticts the usefulness of compression when it comes to .bmp images.

// .bmp file contents

// file header
// image header
// color table
// pixel data

// every Windows BMP begins with a BITMAPFILEHEADER struct
// this helps in recognizing the file format as .bmp

// the first two bytes will be 'B', 'M'

const uint16_t static SOBMP = ((uint16_t) 'M' << 8) | (uint16_t) 'B';

#pragma pack(push, 1)
typedef struct {
        uint16_t SOI;      // BM
        uint32_t FSIZE;
        uint32_t RESERVED; // this is actually two consecutive 16 bit elements, but who cares :)
        uint32_t PIXELDATASTART;
} __BITMAPFILEHEADER;
#pragma pack(pop)

// image header comes in two variants!
// one representing OS/2 BMP format (BITMAPCOREHEADER) and another representing the most common Windows BMP format.
// (BITMAPINFOHEADER)
// however there are no markers to identify the type of the image header present in the bmp image.
// the only way to determine this is to examine the struct's size filed, that is the first 4 bytes of the struct.
// sizeof BITMAPCOREHEADER is 12 bytes
// sizeof BITMAPINFOHEADER is >= 40 bytes.
// from Windows 95 onwards, Windows supports an extended version of BITMAPINFOHEADER, which could be larger than 40 bytes!

// types of compressions used in BMP files.
#pragma pack(push, 4)
typedef enum { RGB, RLE8, RLE4, BITFIELDS } BMPCOMPRESSIONKIND; // uint32_t
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct {
        uint32_t           HEADERSIZE;                          // >= 40 bytes.
        uint32_t           WIDTH;
        int32_t            HEIGHT; // usually an unsigned value, a negative value alludes that the pixel data is ordered top down,
        // instead of the customary bottom up order. bmp images with a - height values may not be compressed!
        uint16_t           NPLANES;       // must be 1
        uint16_t           NBITSPERPIXEL; // 1, 4, 8, 16, 24 or 32
        BMPCOMPRESSIONKIND CMPTYPE;
        uint32_t           IMAGESIZE;     // 0 if not compressed.
        uint32_t           RESPPMX;       // resolution in pixels per meter along x axis.
        uint32_t           RESPPMY;       // resolution in pixels per meter along y axis.
        uint32_t           NCMAPENTRIES;  // number of entries in the colourmap that are used.
        uint32_t           NIMPCOLORS;    // number of important colors.
} __BITMAPINFOHEADER;
#pragma pack(pop)

// a BMP with BITMAPCOREHEADER cannot be compressed.
typedef struct {
        uint32_t HEADERSIZE;    // 12 bytes
        uint16_t WIDTH;
        uint16_t HEIGHT;
        uint16_t NPLANES;       // must be 1
        uint16_t NBITSPERPIXEL; // 1, 4, 8 or 24
} __BITMAPCOREHEADER;

// a colour palette structure immediately follows the BMP file header
// there are 3 variants of the colour palette structure.
// first two variants are used to map pixel data to RGB values, when the bit count is 1, 4 or 8.
// these two variants are common in Windows BMP files.
#pragma pack(push, 1)
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
        uint8_t RESERVED; // must be 0, but seems to be 0xFF in most BMPs, yikes!
} ___RGBQUAD;
#pragma pack(pop)

// BMP files in OS/2 use the third variant
typedef struct {
        uint8_t BLUE;
        uint8_t GREEN;
        uint8_t RED;
} ___RGBTRIPLE;

// A struct representing a BMP image.
typedef struct {
        uint64_t           fsize;
        uint64_t           npixels;
        __BITMAPFILEHEADER fhead;
        __BITMAPINFOHEADER infhead;
        ___RGBQUAD*        pixel_buffer;
} BMP;

static __forceinline BMPCOMPRESSIONKIND __stdcall get_bmp_compression_kind(_In_ const uint32_t cmpkind) {
    switch (cmpkind) {
        case 0 : return RGB;
        case 1 : return RLE8;
        case 2 : return RLE4;
        case 3 : return BITFIELDS;
    }
    return -1;
}

static inline __BITMAPFILEHEADER parse_bitmapfile_header(_In_ const uint8_t* restrict imstream, _In_ const uint64_t fsize) {
    static_assert(sizeof(__BITMAPFILEHEADER) == 14LLU, "Error: __BITMAPFILEHEADER is not 14 bytes in size.");
    assert(fsize >= sizeof(__BITMAPFILEHEADER));

    __BITMAPFILEHEADER header = { 0, 0, 0, 0 };
    // due to little endianness, two serial bytes 0x42, 0x4D will be interpreted as 0x4D42 when casted as
    // an uint16_t yikes!, thereby warranting a little bitshift.
    header.SOI                = (((uint16_t) (*(imstream + 1))) << 8) | ((uint16_t) (*imstream));
    if (header.SOI != SOBMP) {
        fputws(L"Error in parse_bitmapfile_header, file appears not to be a Windows BMP file\n", stderr);
        return header;
    }
    header.FSIZE          = *((uint32_t*) (imstream + 2));
    header.PIXELDATASTART = *((uint32_t*) (imstream + 10));
    return header;
}

static inline __BITMAPINFOHEADER parse_bitmapinfo_header(_In_ const uint8_t* const restrict imstream, _In_ const uint64_t fsize) {
    static_assert(sizeof(__BITMAPINFOHEADER) == 40LLU, "Error: __BITMAPINFOHEADER is not 40 bytes in size");
    assert(fsize >= (sizeof(__BITMAPFILEHEADER) + sizeof(__BITMAPINFOHEADER)));

    __BITMAPINFOHEADER header = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    if (*((uint32_t*) (imstream + 14U)) > 40) {
        fputws(L"BITMAPINFOHEADER larger than 40 bytes! BMP image seems to contain an unparsable file info header", stderr);
        return header;
    }
    header.HEADERSIZE    = *((uint32_t*) (imstream + 14U));
    header.WIDTH         = *((uint32_t*) (imstream + 18U));
    header.HEIGHT        = *((uint32_t*) (imstream + 22U));
    header.NPLANES       = *((uint16_t*) (imstream + 26U));
    header.NBITSPERPIXEL = *((uint16_t*) (imstream + 28U));
    header.CMPTYPE       = get_bmp_compression_kind(*((uint32_t*) (imstream + 30U)));
    header.IMAGESIZE     = *((uint32_t*) (imstream + 34U));
    header.RESPPMX       = *((uint32_t*) (imstream + 38U));
    header.RESPPMY       = *((uint32_t*) (imstream + 42U));
    header.NCMAPENTRIES  = *((uint32_t*) (imstream + 46U));
    header.NIMPCOLORS    = *((uint32_t*) (imstream + 50U));
    return header;
}

// not 100% accurate.
static __forceinline bool __stdcall is_compressed(_In_ const __BITMAPINFOHEADER bmpinfh) { return bmpinfh.IMAGESIZE ? true : false; }

typedef enum { TOPDOWN, BOTTOMUP } BMPPIXDATAORDERING;

static __forceinline BMPPIXDATAORDERING __stdcall get_pixel_order(_In_ const __BITMAPINFOHEADER bmpinfh) {
    if (bmpinfh.HEIGHT >= 0) return BOTTOMUP;
    return TOPDOWN;
}

static inline BMP new_bmp(_In_ wchar_t* file_name) {
    BMP image = {
        .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                .pixel_buffer = NULL
    };

    uint8_t* buffer = open_image(file_name, &image.fsize);
    if (!buffer) {
        // open_image will print the error messages, so no need to do that here.
        wprintf_s(L"Error in %s (%s, %d), open_image returned NULL\n", __FUNCTIONW__, __FILEW__, __LINE__);
        return (BMP) {
            .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                    .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                    .pixel_buffer = NULL
        };
    }

    image.fhead   = parse_bitmapfile_header(buffer, image.fsize);
    image.infhead = parse_bitmapinfo_header(buffer, image.fsize);
    assert(!((image.fsize - 54) % 4)); // Make sure that the number of bytes in the pixel buffer is divisible by 4 without remainders.

    image.npixels      = (image.fsize - 54) / 4;
    image.pixel_buffer = malloc(image.fsize - 54);
    if (!image.pixel_buffer) {         // If malloc failed,
        wprintf_s(L"Error in %s (%s, %d), malloc returned NULL\n", __FUNCTIONW__, __FILEW__, __LINE__);
        free(buffer);
        return (BMP) {
            .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                    .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                    .pixel_buffer = NULL
        };
    }

    // memcyp_s will throw an invalid parameter exception when it finds that the destination size is less than source size!
    // be careful.
    memcpy_s((uint8_t*) image.pixel_buffer, image.fsize - 54, buffer + 54, image.fsize - 54);
    free(buffer);
    return image; // caller needs to free the image.pixel_buffer
}

static inline bool serialize_bmp(_In_ const BMP* const restrict image, _In_ const wchar_t* restrict file_name) {
    HANDLE hfile = CreateFileW(file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return false;
    }

    uint32_t nbyteswritten = 0;
    uint8_t  tmp[54]       = { 0 };
    memcpy_s(tmp, 54, &image->fhead, 14);
    memcpy_s(tmp + 14, 40, &image->infhead, 40);

    if (!WriteFile(hfile, tmp, 54, &nbyteswritten, NULL)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile\n", GetLastError());
        CloseHandle(hfile);
        return false;
    }
    CloseHandle(hfile);

    hfile = CreateFileW(file_name, FILE_APPEND_DATA, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return false;
    }
    if (!WriteFile(hfile, image->pixel_buffer, image->fsize - 54, &nbyteswritten, NULL)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile\n", GetLastError());
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}

static inline void print_bmp_info(_In_ const BMP* const restrict image) {
    wprintf_s(
        L"Start marker: 424D\nFile size %Lf MiBs\nPixel data start offset: %d\n",
        ((long double) image->fhead.FSIZE) / (1024 * 1024U),
        image->fhead.PIXELDATASTART
    );
    wprintf_s(
        L"BITMAPINFOHEADER size: %u\nImage width: %u\nImage height: %u\nNumber of planes: %hu\n"
        L"Number of bits per pixel: %hu\nImage size: %u\nResolution PPM(X): %u\nResolution PPM(Y): %u\nNumber of used colormap entries: %u\n"
        L"Number of important colors: %u\n",
        image->infhead.HEADERSIZE,
        image->infhead.WIDTH,
        image->infhead.HEIGHT,
        image->infhead.NPLANES,
        image->infhead.NBITSPERPIXEL,
        image->infhead.IMAGESIZE,
        image->infhead.RESPPMX,
        image->infhead.RESPPMY,
        image->infhead.NCMAPENTRIES,
        image->infhead.NIMPCOLORS
    );
    switch (image->infhead.CMPTYPE) {
        case RGB       : _putws(L"BITMAPINFOHEADER.CMPTYPE: RGB"); break;
        case RLE4      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE4"); break;
        case RLE8      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE8"); break;
        case BITFIELDS : _putws(L"BITMAPINFOHEADER.CMPTYPE: BITFIELDS"); break;
    }

    wprintf_s(L"%s BMP file\n", is_compressed(image->infhead) ? L"Compressed" : L"Uncompressed");
    wprintf_s(L"BMP pixel ordering: %s\n", get_pixel_order(image->infhead) ? L"BOTTOMUP" : L"TOPDOWN");
    return;
}

// en enum to specify the RGB -> BW conversion method.
// AVERAGE takes the mean of R, G and B values.
// WEIGHTED_AVERAGE does GREY = (0.299 R) + (0.587 G) + (0.114 B)
// LUMINOSITY does LUM = (0.2126 R) + (0.7152 G) + (0.0722 B)
typedef enum { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY } RGBTOBWKIND;

// If inplace = true, return value can be safely ignored.
static inline BMP to_blacknwhite(_In_ const BMP* image, _In_ const RGBTOBWKIND conversion_kind, _In_ const bool inplace) {
    // ___RGBQUAD encoding assumed.
    BMP local = *image;
    if (!inplace) {
        local.pixel_buffer = malloc(image->fsize - 54);
        if (!local.pixel_buffer) {
            wprintf_s(L"Error in %s (%s, %d), malloc returned NULL\n", __FUNCTIONW__, __FILEW__, __LINE__);
            return (BMP) {
                .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                        .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                        .pixel_buffer = NULL
            };
        }
        memcpy_s(local.pixel_buffer, local.fsize - 54, image->pixel_buffer, image->fsize - 54);
    }

    switch (conversion_kind) {
        case AVERAGE :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =
                    ((local.pixel_buffer[i].BLUE + local.pixel_buffer[i].GREEN + local.pixel_buffer[i].RED) / 3); // plain arithmetic mean
            }
            break;
        case WEIGHTED_AVERAGE :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =            // weighted average
                    (uint8_t) ((local.pixel_buffer[i].BLUE * 0.299L) + (local.pixel_buffer[i].GREEN * 0.587L) +
                               (local.pixel_buffer[i].RED * 0.114L));
            }
            break;
        case LUMINOSITY :
            for (size_t i = 0; i < local.npixels; ++i) {
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED =
                    (uint8_t) ((local.pixel_buffer[i].BLUE * 0.2126L) + (local.pixel_buffer[i].GREEN * 0.7152L) +
                               (local.pixel_buffer[i].RED * 0.0722L));
            }
            break;
        // case BINARY :
        //     for (size_t i = 0; i < local.npixels; ++i) {
        //         local.pixel_buffer[i].BLUE  = local.pixel_buffer[i].BLUE > 128 ? 255 : 0;
        //         local.pixel_buffer[i].GREEN = local.pixel_buffer[i].GREEN > 128 ? 255 : 0;
        //         local.pixel_buffer[i].RED   = local.pixel_buffer[i].RED > 128 ? 255 : 0;
        //     }
        //     break;
        case BINARY :
            uint64_t value = 0;
            for (size_t i = 0; i < local.npixels; ++i) {
                value                      = (local.pixel_buffer[i].BLUE + local.pixel_buffer[i].GREEN + local.pixel_buffer[i].RED);
                local.pixel_buffer[i].BLUE = local.pixel_buffer[i].GREEN = local.pixel_buffer[i].RED = (value / 3) > 128 ? 255 : 0;
            }
            break;
    }
    return local;
}

// The color palette in pixel buffer is in BGR order NOT RGB!
typedef enum { REMRED, REMGREEN, REMBLUE, REMRG, REMRB, REMGB } RMCOLOURKIND;

// If inplace = true, return value can be safely ignored.
static inline BMP remove_color(_In_ const BMP* image, _In_ const RMCOLOURKIND rmcolor, _In_ const bool inplace) {
    BMP local = *image;
    if (!inplace) {
        local.pixel_buffer = malloc(image->fsize - 54);
        if (!local.pixel_buffer) {
            wprintf_s(L"Error in %s (%s, %d), malloc returned NULL\n", __FUNCTIONW__, __FILEW__, __LINE__);
            return (BMP) {
                .fsize = 0, .npixels = 0, .fhead = { 0, 0, 0, 0 },
                        .infhead = { 0, 0, 0, 0, 0, RGB, 0, 0, 0, 0, 0 },
                        .pixel_buffer = NULL
            };
        }
        memcpy_s(local.pixel_buffer, local.fsize - 54, image->pixel_buffer, image->fsize - 54);
    }

    switch (rmcolor) {
        case REMRED :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = 0;
            break;
        case REMGREEN :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].GREEN = 0;
            break;
        case REMBLUE :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].BLUE = 0;
            break;
        case REMRG :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = local.pixel_buffer[i].GREEN = 0;
            break;
        case REMRB :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].RED = local.pixel_buffer[i].BLUE = 0;
            break;
        case REMGB :
            for (size_t i = 0; i < local.npixels; ++i) local.pixel_buffer[i].GREEN = local.pixel_buffer[i].BLUE = 0;
            break;
    }
    return local;
}

static void __forceinline __stdcall close_bmp(_In_ const BMP image) {
    free(image.pixel_buffer);
    return;
}

int wmain(_In_ int argc, _In_ wchar_t* argv[]) {
    if (argc < 2) {
        fputs("Error: no files passed, bmp.exe takes/needs one argument: [file name]", stderr);
        exit(1);
    }

    BMP image  = new_bmp(argv[1]);

    BMP bw_ave = to_blacknwhite(&image, AVERAGE, false);
    BMP bw_wav = to_blacknwhite(&image, WEIGHTED_AVERAGE, false);
    BMP bw_lum = to_blacknwhite(&image, LUMINOSITY, false);
    BMP bw_bin = to_blacknwhite(&image, BINARY, false);

    serialize_bmp(&bw_ave, L"./sydney_bw_ave.bmp");
    serialize_bmp(&bw_wav, L"./sydney_bw_wav.bmp");
    serialize_bmp(&bw_lum, L"./sydney_bw_lum.bmp");
    serialize_bmp(&bw_bin, L"./sydney_bw_bin.bmp");

    close_bmp(bw_ave);
    close_bmp(bw_wav);
    close_bmp(bw_lum);
    close_bmp(bw_bin);

    BMP nored   = remove_color(&image, REMRED, false);
    BMP nogreen = remove_color(&image, REMGREEN, false);
    BMP noblue  = remove_color(&image, REMBLUE, false);
    BMP norg    = remove_color(&image, REMRG, false);
    BMP norb    = remove_color(&image, REMRB, false);
    BMP nogb    = remove_color(&image, REMGB, false);

    serialize_bmp(&nored, L"./sydney_GB.bmp");
    serialize_bmp(&noblue, L"./sydney_RG.bmp");
    serialize_bmp(&nogreen, L"./sydney_RB.bmp");
    serialize_bmp(&nogb, L"./sydney_R.bmp");
    serialize_bmp(&norb, L"./sydney_G.bmp");
    serialize_bmp(&norg, L"./sydney_B.bmp");

    close_bmp(image);
    close_bmp(nored);
    close_bmp(noblue);
    close_bmp(nogreen);
    close_bmp(nogb);
    close_bmp(norb);
    close_bmp(norg);

    return EXIT_SUCCESS;
}