#include <ico.hpp>

// make sure caller validates that `bytestream` is not empty before calling ParseIconDir()
[[nodiscard]] static ico::ICONDIR ParseIconDir(_In_ const std::vector<uint8_t>& bytestream) noexcept {
    assert(!bytestream.empty()); // won't help in release builds, caller is responsible!
    static constexpr auto tmp { ico::ICONDIR {} };

    const auto            idReserved = *reinterpret_cast<const uint16_t*>(bytestream.data());
    if (idReserved) { // must be 0
        std::wcerr << L"Non zero value encountered for idReserved in ParseIconDir()!\n";
        return tmp;
    }

    const auto idType = static_cast<ico::IMAGETYPE>(*reinterpret_cast<const uint16_t*>(bytestream.data() + 2));
    if (idType != ico::IMAGETYPE::ICO && idType != ico::IMAGETYPE::CUR) { // cannot be anything else
        std::wcerr << L"File found not to be of type ICON or CURSOR in ParseIconDir()!\n";
        return tmp;
    }

    const auto idCount = *reinterpret_cast<const uint16_t*>(bytestream.data() + 4);

    return { .idReserved = idReserved, .idType = idType, .idCount = idCount };
}

[[nodiscard]] static std::array<ico::ICONDIRENTRY, ico::MAX_ICONDIRENTRIES> ParseIconDirEntries(
    _In_ const std::vector<uint8_t> bytestream, _In_ const size_t nentries /* number of ICONDIRENTRY structs present in bytestream */
) noexcept {
    assert(nentries > 0);
    const uint8_t* const parse_start_pos { bytestream.data() + 6 }; // 6 bytes offset for 3 WORDS - idReserved, idType & idCount.
}

ico::ico::ico(_In_ const wchar_t* const filename) noexcept {
    const auto fbuffer { io::Open(filename) };
    return;
}
