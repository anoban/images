#include "bmp.hpp"

std::vector<uint8_t> bmp::open(_In_ const std::wstring& path, _Out_ uint64_t* const nread_bytes) {
    *nread_bytes                = 0;
    HANDLE               handle = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);
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

    return buff;
}

bmp::BITMAPFILEHEADER bmp::bmp::parse_fileheader(_In_ const std::vector<uint8_t>& imstream) {
    static_assert(sizeof(BITMAPFILEHEADER) == 14LLU, "Error: BITMAPFILEHEADER must be 14 bytes in size");
    assert(imstream.size() >= sizeof(BITMAPFILEHEADER));

    BITMAPFILEHEADER header {};
    const auto       tmp {
        std::array<uint8_t, 2> {imstream.at(0), imstream.at(1)}
    };
    if (tmp != soi) throw std::runtime_error("Error in bmp::bmp::parse_fileheader, file isn't a Windows BMP file\n");
    header.FSIZE          = *reinterpret_cast<const uint32_t*>(imstream.data() + 2);
    header.PIXELDATASTART = *reinterpret_cast<const uint32_t*>(imstream.data() + 10);

    return header;
}

bmp::BITMAPINFOHEADER bmp::bmp::parse_infoheader(_In_ const std::vector<uint8_t>& imstream) {
    static_assert(sizeof(BITMAPINFOHEADER) == 40LLU, "Error: BITMAPINFOHEADER must be 40 bytes in size");
    assert(imstream.size() >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)));

    BITMAPINFOHEADER header = { 0, 0, 0, 0, 0, COMPRESSIONKIND::UNKNOWN, 0, 0, 0, 0, 0 };
    if (*reinterpret_cast<const uint32_t*>(imstream.data() + 14U) > 40)
        throw std::runtime_error("Error: Cannot parse BITMAPINFOHEADERs larger than 40 bytes");

    header.HEADERSIZE    = *reinterpret_cast<const uint32_t*>(imstream.data() + 14);
    header.WIDTH         = *reinterpret_cast<const uint32_t*>(imstream.data() + 18);
    header.HEIGHT        = *reinterpret_cast<const uint32_t*>(imstream.data() + 22);
    header.NPLANES       = *reinterpret_cast<const uint16_t*>(imstream.data() + 26);
    header.NBITSPERPIXEL = *reinterpret_cast<const uint16_t*>(imstream.data() + 28);
    header.CMPTYPE       = get_bmp_compression_kind(*((uint32_t*) (imstream.data() + 30U)));
    header.IMAGESIZE     = *reinterpret_cast<const uint32_t*>(imstream.data() + 34);
    header.RESPPMX       = *reinterpret_cast<const uint32_t*>(imstream.data() + 38);
    header.RESPPMY       = *reinterpret_cast<const uint32_t*>(imstream.data() + 42);
    header.NCMAPENTRIES  = *reinterpret_cast<const uint32_t*>(imstream.data() + 46);
    header.NIMPCOLORS    = *reinterpret_cast<const uint32_t*>(imstream.data() + 50);
    return header;
}