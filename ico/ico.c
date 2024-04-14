#include <ico.h>

// make sure caller validates that `bytestream` is not empty before calling ParseIconDir()
static inline ICONDIR ParseIconDir(_In_ const uint8_t* const bytestream, _In_ const size_t length) {
    assert(!bytestream); // won't help in release builds, caller is responsible!

    static ICONDIR tmp        = { 0 };

    const auto     idReserved = *((uint16_t*) (bytestream));
    if (idReserved) { // must be 0
        std::wcerr << L"Error in ParseIconDir():: Non zero value encountered for idReserved!\n";
        return tmp;
    }

    const uint16_t idType = *((uint16_t*) (bytestream + 2));
    if (idType != IMAGETYPE::ICO && idType != IMAGETYPE::CUR) { // cannot be anything else
        std::wcerr << L"Error in ParseIconDir():: File found not to be of type ICON or CURSOR!\n";
        return tmp;
    }

    const auto idCount = *reinterpret_cast<const uint16_t*>(bytestream + 4);

    return { .idReserved = idReserved, .idType = idType, .idCount = idCount };
}

// bytestream buffer needs to be correctly offsetted to the parse start point
[[nodiscard]] static ICONDIRENTRY ParseIconDirEntry(_In_ const uint8_t* const bytestream /* where the parsing begins */) noexcept {
    assert(bytestream); // make sure this is valid
    auto tmp { ICONDIRENTRY {} };
    static_assert(sizeof(ICONDIRENTRY) == 16);

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

[[nodiscard]] static std::array<ICONDIRENTRY, MAX_ICONDIRENTRIES> ParseIconDirEntries(
    _In_ const std::vector<uint8_t>& bytestream, _In_ const size_t nentries /* number of ICONDIRENTRY structs present in bytestream */
) noexcept {
    assert(!bytestream.empty());
    assert((nentries > 0) && (nentries <= MAX_ICONDIRENTRIES));

    std::array<ICONDIRENTRY, MAX_ICONDIRENTRIES> tmp {};

    const uint8_t* const parse_start_pos { bytestream + 6 }; // 6 bytes offset for 3 WORDS - idReserved, idType & idCount.
    for (int32_t i {}; i < nentries; ++i) tmp.at(i) = ParseIconDirEntry(parse_start_pos + i * sizeof(ICONDIRENTRY));
    return tmp;
}

} // namespace helpers

// check whether an ICONDIRENTRY struct is in it's default initialized state
bool ICONDIRENTRY::is_empty() const noexcept {
    return sizeof(ICONDIRENTRY) ==
           std::count(reinterpret_cast<const uint8_t*>(this), reinterpret_cast<const uint8_t*>(this + sizeof(ICONDIRENTRY)), 0);
}

ico(_In_ const wchar_t* const filename) {
    auto fbuffer { io::Open(filename) };
    if (!fbuffer.has_value()) {
        std::wcerr << L"Error in ico()!\n";
        return;
    }

    assert(!fbuffer.value().empty()); // make sure the vector ain't empty
    buffer      = std::move(fbuffer.value());
    icDirectory = ::ParseIconDir(buffer);
    icdEntries  = ::ParseIconDirEntries(buffer, icDirectory.idCount);
    return;
}

template<typename T> requires ::is_printable<T> std::basic_ostream<T>& operator<<(std::basic_ostream<T>& ostr, const ico& image) { }
