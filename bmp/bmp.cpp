#include "bmp.hpp"

std::vector<uint8_t> bmp::open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes) {
    *nread_bytes = 0;
    HANDLE               handle { ::CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr) };
    std::vector<uint8_t> buff {};

    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));

    LARGE_INTEGER size;
    if (!::GetFileSizeEx(handle, &size)) {
        ::CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in GetFileSizeEx [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));
    }

    buff.resize(static_cast<uint64_t>(size.QuadPart));
    // allocation errors will be automatically handled by the C++ runtime.

    if (!::ReadFile(handle, buff.data(), static_cast<DWORD>(size.QuadPart), reinterpret_cast<LPDWORD>(nread_bytes), nullptr)) {
        ::CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in ReadFile [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));
    }

    ::CloseHandle(handle);
    return buff;
}

std::vector<std::wstring> bmp::remove_ext(
    _In_count_(size) wchar_t* const fnames[], _In_ const size_t length, _In_ const wchar_t* const extension
) noexcept {
    std::vector<std::wstring> trimmed {};
    for (size_t i = 1; i < length; ++i) {
        std::wstring tmp { fnames[i] };
        // if the string contains multiple instances of the extension, all the characters following the first instance will be erased
        // e.g. "dell_alienware_hd.bmp.bwhite.bmp" will become dell_alienware_hd is the extension is ".bmp"
        size_t       pos {};
        if ((pos = tmp.find(extension)) != std::wstring::npos) trimmed.push_back(std::move(tmp.substr(0, pos)));
    }
    return trimmed;
}

bmp::BITMAPFILEHEADER bmp::bmp::parse_fileheader(_In_ const std::vector<uint8_t>& imstream) {
    static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER must be 14 bytes in size");
    assert(imstream.size() >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER             header {};
    const std::array<uint8_t, 2> tmp {
        {imstream.at(0), imstream.at(1)}
    };
    if (tmp != soi) throw std::runtime_error("Error in bmp::bmp::parse_fileheader, file isn't a Windows BMP file\n");
    header.SOI            = soi;
    header.FSIZE          = *reinterpret_cast<const uint32_t*>(imstream.data() + 2);
    header.PIXELDATASTART = *reinterpret_cast<const uint32_t*>(imstream.data() + 10);

    return header;
}

bmp::COMPRESSIONKIND bmp::bmp::get_compressionkind(_In_ const uint32_t cmpkind) noexcept {
    switch (cmpkind) {
        [[likely]] case 0 :
            return COMPRESSIONKIND::RGB;
        case 1 : return COMPRESSIONKIND::RLE8;
        case 2 : return COMPRESSIONKIND::RLE4;
        case 3 : return COMPRESSIONKIND::BITFIELDS;
    }
    return COMPRESSIONKIND::UNKNOWN;
}

bmp::BITMAPINFOHEADER bmp::bmp::parse_infoheader(_In_ const std::vector<uint8_t>& imstream) {
    static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "Error: BITMAPINFOHEADER must be 40 bytes in size");
    assert(imstream.size() >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

    BITMAPINFOHEADER header { 0, 0, 0, 0, 0, COMPRESSIONKIND::UNKNOWN, 0, 0, 0, 0, 0 };
    if (*reinterpret_cast<const uint32_t*>(imstream.data() + 14U) > 40) throw std::runtime_error("Error: Unparseable BITMAPINFOHEADER\n");

    header.HEADERSIZE    = *reinterpret_cast<const uint32_t*>(imstream.data() + 14);
    header.WIDTH         = *reinterpret_cast<const uint32_t*>(imstream.data() + 18);
    header.HEIGHT        = *reinterpret_cast<const int32_t*>(imstream.data() + 22);
    header.NPLANES       = *reinterpret_cast<const uint16_t*>(imstream.data() + 26);
    header.NBITSPERPIXEL = *reinterpret_cast<const uint16_t*>(imstream.data() + 28);
    header.CMPTYPE       = get_compressionkind(*reinterpret_cast<const uint32_t*>(imstream.data() + 30U));
    header.IMAGESIZE     = *reinterpret_cast<const uint32_t*>(imstream.data() + 34);
    header.RESPPMX       = *reinterpret_cast<const uint32_t*>(imstream.data() + 38);
    header.RESPPMY       = *reinterpret_cast<const uint32_t*>(imstream.data() + 42);
    header.NCMAPENTRIES  = *reinterpret_cast<const uint32_t*>(imstream.data() + 46);
    header.NIMPCOLORS    = *reinterpret_cast<const uint32_t*>(imstream.data() + 50);

    return header;
}

bmp::BMPPIXDATAORDERING bmp::bmp::get_pixelorder(_In_ const BITMAPINFOHEADER& header) noexcept {
    return (header.HEIGHT >= 0) ? BMPPIXDATAORDERING::BOTTOMUP : BMPPIXDATAORDERING::TOPDOWN;
}

void bmp::bmp::serialize(_In_ const std::wstring& path) {
    HANDLE handle { ::CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) };

    if (handle == INVALID_HANDLE_VALUE) {
        ::CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));
    }

    DWORD                   nbytes {};
    std::array<uint8_t, 54> tmp {};
    // following line assumes that std::array<uint8_t, 2> has the same memory layout as uint8_t buff[2]
    // if std::array has a skeleton like std::vector, this might cause problems.
    std::memcpy(tmp.data(), &fh, sizeof(BITMAPFILEHEADER));
    std::memcpy(tmp.data() + 14, &infh, sizeof(BITMAPINFOHEADER));

    if (!::WriteFile(handle, tmp.data(), 54, &nbytes, nullptr)) {
        ::CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in WriteFile [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));
    }
    ::CloseHandle(handle);

    handle = ::CreateFileW(path.c_str(), FILE_APPEND_DATA, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));

    if (!::WriteFile(handle, pixels.data(), pixels.size() * sizeof(RGBQUAD), &nbytes, nullptr)) {
        ::CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in WriteFile [{}{:4d}]\n", ::GetLastError(), __FILE__, __LINE__));
    }

    ::CloseHandle(handle);
    return;
}

bmp::bmp::bmp(_In_ const std::wstring& path) {
    try {
        [[msvc::forceinline_calls]] auto fbuff { open(path, &size) };
        [[msvc::forceinline_calls]] fh   = parse_fileheader(fbuff);
        [[msvc::forceinline_calls]] infh = parse_infoheader(fbuff);

        npixels                          = (size - 54) / 4;
        for (auto it = fbuff.begin() + 54; it != fbuff.cend(); it += 4) pixels.push_back(RGBQUAD { *it, *(it + 1), *(it + 2), *(it + 3) });
    } catch (const std::exception& excpt) { throw std::exception(excpt); }
}

constexpr bmp::bmp::bmp(
    _In_ const BITMAPFILEHEADER& headf, _In_ const BITMAPINFOHEADER& headinf, _In_ const std::vector<RGBQUAD>& pbuff
) noexcept :
    fh { headf }, infh { headinf }, pixels { std::move(pbuff) } { }

void bmp::bmp::info(void) noexcept {
    ::wprintf_s(
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
        [[likely]] case COMPRESSIONKIND::RGB :
            _putws(L"BITMAPINFOHEADER.CMPTYPE: RGB");
            break;
        case COMPRESSIONKIND::RLE4      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE4"); break;
        case COMPRESSIONKIND::RLE8      : _putws(L"BITMAPINFOHEADER.CMPTYPE: RLE8"); break;
        case COMPRESSIONKIND::BITFIELDS : _putws(L"BITMAPINFOHEADER.CMPTYPE: BITFIELDS"); break;
        case COMPRESSIONKIND::UNKNOWN   : _putws(L"BITMAPINFOHEADER.CMPTYPE: UNKNOWN"); break;
    }

    ::wprintf_s(
        L"%s BMP file\n"
        L"BMP pixel ordering: %s\n",
        infh.IMAGESIZE != 0 ? L"Compressed" : L"Uncompressed",
        get_pixelorder(infh) == BMPPIXDATAORDERING::BOTTOMUP ? L"BOTTOMUP" : L"TOPDOWN"
    );

    return;
}

std::optional<bmp::bmp> bmp::bmp::tobwhite(_In_ const TOBWKIND cnvrsnkind, _In_opt_ const bool inplace) noexcept {
    bmp* imptr { nullptr };
    bmp  copy {};
    if (inplace)
        imptr = this;
    else {
        [[likely]] copy = *this; // copy the bmp instance
        imptr           = &copy;
    }

    switch (cnvrsnkind) {
        case TOBWKIND::AVERAGE :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((static_cast<long double>(pix.BLUE) + pix.GREEN + pix.RED) / 3);
            });
            break;

        case TOBWKIND::WEIGHTED_AVERAGE :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.299L) + (pix.GREEN * 0.587) + (pix.RED * 0.114));
            });
            break;

        case TOBWKIND::LUMINOSITY :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = static_cast<uint8_t>((pix.BLUE * 0.2126L) + (pix.GREEN * 0.7152L) + (pix.RED * 0.0722L));
            });
            break;

        case TOBWKIND::BINARY :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
                pix.BLUE = pix.GREEN = pix.RED = (static_cast<uint64_t>(pix.BLUE) + pix.GREEN + pix.RED) / 3 >= 128 ? 255Ui8 : 0Ui8;
            });
            break;
    }

    if (inplace)
        return std::nullopt;
    else [[likely]]
        return copy;
}

std::optional<bmp::bmp> bmp::bmp::tonegative(_In_opt_ const bool inplace) noexcept {
    bmp* imptr { nullptr };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) {
        pix.BLUE  = pix.BLUE >= 128 ? 255Ui8 : 0Ui8;
        pix.GREEN = pix.GREEN >= 128 ? 255Ui8 : 0Ui8;
        pix.RED   = pix.RED >= 128 ? 255Ui8 : 0Ui8;
    });

    if (inplace)
        return std::nullopt;
    else [[likely]]
        return copy;
}

std::optional<bmp::bmp> bmp::bmp::remove_clr(_In_ const RGBCOMB kind, _In_opt_ const bool inplace) noexcept {
    bmp* imptr { nullptr };
    bmp  copy {};
    if (inplace) {
        imptr = this;
    } else {
        [[likely]] copy = *this;
        imptr           = &copy;
    }

    switch (kind) {
        case RGBCOMB::BLUE  : std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.BLUE = 0; }); break;
        case RGBCOMB::GREEN : std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = 0; }); break;
        case RGBCOMB::RED   : std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = 0; }); break;
        case RGBCOMB::REDGREEN :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.GREEN = 0; });
            break;
        case RGBCOMB::REDBLUE :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.RED = pix.BLUE = 0; });
            break;
        case RGBCOMB::GREENBLUE :
            std::for_each(imptr->pixels.begin(), imptr->pixels.end(), [](_In_ RGBQUAD& pix) { pix.GREEN = pix.BLUE = 0; });
            break;
    }
    if (inplace)
        return std::nullopt;
    else
        return copy;
}

// TODO: Implementation works fine only when width and height are divisible by 256 without remainders. SORT THIS OUT!
// DO NOT repeat the static keyword here! It's enough to declare the method as static only in the header file.
bmp::bmp bmp::bmp::gengradient(_In_opt_ const size_t heightpx, _In_opt_ const size_t widthpx) noexcept {
    assert(heightpx > 255 && widthpx > 255);

    BITMAPFILEHEADER file {
        .SOI { std::array<uint8_t, 2> { { 'B', 'M' } } },
        .FSIZE          = static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * heightpx * widthpx),
        .PIXELDATASTART = 54
    };

    BITMAPINFOHEADER     info { .HEADERSIZE    = 40,
                                .WIDTH         = static_cast<uint32_t>(widthpx),
                                .HEIGHT        = static_cast<int32_t>(heightpx),
                                .NPLANES       = 1,
                                .NBITSPERPIXEL = 32,
                                .CMPTYPE       = COMPRESSIONKIND::RGB,
                                .IMAGESIZE     = 0,
                                .RESPPMX       = 0,
                                .RESPPMY       = 0,
                                .NCMAPENTRIES  = 0,
                                .NIMPCOLORS    = 0 };

    std::vector<RGBQUAD> pixels {};
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