#include "bmp.hpp"

[[nodiscard]] std::vector<uint8_t> bmp::open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes) {
    *nread_bytes = 0;
    HANDLE               handle { CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr) };
    std::vector<uint8_t> buff {};

    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));

    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size)) {
        CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in GetFileSizeEx [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    buff.resize(size.QuadPart);
    // allocation errors will be automatically handled by the C++ runtime.

    if (!ReadFile(handle, buff.data(), size.QuadPart, reinterpret_cast<LPDWORD>(nread_bytes), nullptr)) {
        CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in ReadFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    CloseHandle(handle);
    return buff;
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
        case 0 : return COMPRESSIONKIND::RGB;
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
    header.HEIGHT        = *reinterpret_cast<const uint32_t*>(imstream.data() + 22);
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

bmp::BMPPIXDATAORDERING bmp::bmp::get_pixelorder(_In_ const BITMAPINFOHEADER& infh) noexcept {
    return (infh.HEIGHT >= 0) ? BMPPIXDATAORDERING::BOTTOMUP : BMPPIXDATAORDERING::TOPDOWN;
}

void bmp::bmp::serialize(_In_ const std::wstring& path) {
    HANDLE handle { CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) };

    if (handle == INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    DWORD                   nbytes {};
    std::array<uint8_t, 54> tmp {};
    std::memcpy(tmp.data(), &fh, sizeof(BITMAPFILEHEADER));
    std::memcpy(tmp.data() + 14, &infh, sizeof(BITMAPINFOHEADER));

    if (!WriteFile(handle, tmp.data(), 54, &nbytes, nullptr)) {
        CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in WriteFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }
    CloseHandle(handle);

    handle = CreateFileW(path.c_str(), FILE_APPEND_DATA, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error(std::format("Error {:4d} in CreateFileW [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));

    if (!WriteFile(handle, pixels.data(), pixels.size() * sizeof(RGBQUAD), &nbytes, nullptr)) {
        CloseHandle(handle);
        throw std::runtime_error(std::format("Error {:4d} in WriteFile [{}{:4d}]\n", GetLastError(), __FILE__, __LINE__));
    }

    CloseHandle(handle);
    return;
}

[[nodiscard]] bmp::bmp::bmp(_In_ const std::wstring& path) {
    try {
        auto fbuff { open(path, &size) };
        fh      = parse_fileheader(fbuff);
        infh    = parse_infoheader(fbuff);
        npixels = (size - 54) / 4;
        for (auto it = fbuff.begin() + 54; it != fbuff.cend(); it += 4) pixels.push_back(RGBQUAD { *it, *(it + 1), *(it + 2), *(it + 3) });
    } catch (const std::exception& excpt) { throw std::exception(excpt); }
}