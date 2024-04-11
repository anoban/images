#include <ico.hpp>

// check whether an ICONDIRENTRY struct is in it's default initialized state
bool ico::ICONDIRENTRY::is_empty() const noexcept {
    return sizeof(ICONDIRENTRY) ==
           std::count(reinterpret_cast<const uint8_t*>(this), reinterpret_cast<const uint8_t*>(this + sizeof(ICONDIRENTRY)), 0);
}

// make sure caller validates that `bytestream` is not empty before calling ParseIconDir()
[[nodiscard]] static ico::ICONDIR ParseIconDir(_In_ const std::vector<uint8_t>& bytestream) {
    assert(!bytestream.empty()); // won't help in release builds, caller is responsible!

    static constexpr auto tmp { ico::ICONDIR {} };

    const auto            idReserved = *reinterpret_cast<const uint16_t*>(bytestream.data());
    if (idReserved) { // must be 0
        std::wcerr << L"Error in ParseIconDir():: Non zero value encountered for idReserved!\n";
        return tmp;
    }

    const auto idType = static_cast<ico::IMAGETYPE>(*reinterpret_cast<const uint16_t*>(bytestream.data() + 2));
    if (idType != ico::IMAGETYPE::ICO && idType != ico::IMAGETYPE::CUR) { // cannot be anything else
        std::wcerr << L"Error in ParseIconDir():: File found not to be of type ICON or CURSOR!\n";
        return tmp;
    }

    const auto idCount = *reinterpret_cast<const uint16_t*>(bytestream.data() + 4);

    return { .idReserved = idReserved, .idType = idType, .idCount = idCount };
}

// bytestream buffer needs to be correctly offsetted to the parse start point
[[nodiscard]] static ico::ICONDIRENTRY ParseIconDirEntry(_In_ const uint8_t* const bytestream /* where the parsing begins */) noexcept {
    assert(bytestream); // make sure this is valid
    auto tmp { ico::ICONDIRENTRY {} };
    static_assert(sizeof(ico::ICONDIRENTRY) == 16);

    tmp.bWidth        = *bytestream;
    tmp.bHeight       = *(bytestream + 1);
    tmp.bColorCount   = *(bytestream + 2);
    tmp.bReserved     = *(bytestream + 3);
    tmp.wPlanes       = *reinterpret_cast<const unsigned short*>(bytestream + 4);
    tmp.wBitCount     = *reinterpret_cast<const unsigned short*>(bytestream + 6);
    tmp.dwBytesInRes  = *reinterpret_cast<const unsigned long*>(bytestream + 8);
    tmp.dwImageOffset = *reinterpret_cast<const unsigned long*>(bytestream + 12);

    return tmp;
}

[[nodiscard]] static std::array<ico::ICONDIRENTRY, ico::MAX_ICONDIRENTRIES> ParseIconDirEntries(
    _In_ const std::vector<uint8_t>& bytestream, _In_ const size_t nentries /* number of ICONDIRENTRY structs present in bytestream */
) noexcept {
    assert(!bytestream.empty());
    assert((nentries > 0) && (nentries <= ico::MAX_ICONDIRENTRIES));

    std::array<ico::ICONDIRENTRY, ico::MAX_ICONDIRENTRIES> tmp {};

    const uint8_t* const parse_start_pos { bytestream.data() + 6 }; // 6 bytes offset for 3 WORDS - idReserved, idType & idCount.
    for (int32_t i {}; i < nentries; ++i) tmp.at(i) = ParseIconDirEntry(parse_start_pos + i * sizeof(ico::ICONDIRENTRY));
    return tmp;
}

ico::ico::ico(_In_ const wchar_t* const filename) {
    const auto fbuffer { io::Open(filename) };
    if (!fbuffer.has_value()) {
        std::wcerr << L"Error in ico::ico()!\n";
        return;
    }

    assert(!fbuffer.value().empty()); // make sure the vector ain't empty
}
