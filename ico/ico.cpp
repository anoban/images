#include <ico.hpp>

static inline ico::ICONDIR ParseIconDirectory(_In_ const std::vector<uint8_t>& bytestream) noexcept {
    // make sure the input vector is not empty before calling the parser
    ico::ICONDIR placeholder {};
    placeholder.idReserved = *reinterpret_cast<const uint16_t*>(bytestream.data());
    if (placeholder.idReserved) { } // must be 0

    placeholder.idType = static_cast<ico::IMAGETYPE>(*reinterpret_cast<const uint16_t*>(bytestream.data() + 2));
    if (placeholder.idType != ico::IMAGETYPE::ICO && placeholder.idType != ico::IMAGETYPE::CUR) { }

    placeholder.idCount = *reinterpret_cast<const uint16_t*>(bytestream.data() + 4);

    return;
}

static inline ico::ICONDIRENTRY ParseIconDirEntry(_In_ const std::vector<uint8_t>& bytestream) noexcept { }

ico::ico::ico(_In_ const wchar_t* const filename) noexcept {
    const auto fbuffer { io::Open(filename) };
    return;
}
