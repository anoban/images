#include <ico.h>

// make sure caller validates that `bytestream` is not empty before calling ParseIconDir()
static __forceinline ICONDIR __stdcall ParseIconDir(_In_ const uint8_t* const restrict bytestream) {
    assert(bytestream); // won't help in release builds, caller is responsible!

    ICONDIR        tmp = { 0 }; // making this static could improve performance but residuals from previous calls might affect the return
    // values in subsequent invocations :(

    const uint16_t idReserved = *((uint16_t*) (bytestream));
    if (idReserved) { // must be 0
        fputws(L"Error in ParseIconDir():: Non zero value encountered for idReserved!", stderr);
        return tmp;
    }

    const uint16_t idType = *((uint16_t*) (bytestream + 2));
    if (idType != ICO && idType != CUR) { // cannot be anything else
        fputws(L"Error in ParseIconDir():: File found not to be of type ICON or CURSOR!", stderr);
        return tmp;
    }

    const uint16_t idCount = *((const uint16_t*) (bytestream + 4));
    // we're 6 bytes past the beginning of the buffer now.

    // handle if the file contains more bitmaps than MAX_ICONDIRENTRIES
    if (idCount > MAX_ICONDIRENTRIES) { }

    for (size_t i = 0; i < idCount; ++i) // taking adavantage of tmp here, instead of declaring and using a new array of ICONDIRENTRYs
        tmp.idEntries[i] = ParseIconDirEntry(bytestream + 6 /* first three words */ + i * sizeof(ICONDIRENTRY));

    return (ICONDIR) { .idReserved = idReserved, .idType = idType, .idCount = idCount, .idEntries = tmp.idEntries };
}

// bytestream buffer needs to be correctly offsetted to the parse start point
static __forceinline ICONDIRENTRY __stdcall ParseIconDirEntry(_In_ const uint8_t* const restrict bytestream /* where the parsing begins */
) {
    assert(bytestream); // make sure this is valid
    static ICONDIRENTRY tmp = { 0 };

    tmp.bWidth              = *bytestream;
    tmp.bHeight             = *(bytestream + 1);
    tmp.bColorCount         = *(bytestream + 2);
    tmp.bReserved           = *(bytestream + 3);
    assert(!tmp.bReserved); // must always be 0

    tmp.wPlanes       = *((unsigned short*) (bytestream + 4));
    tmp.wBitCount     = *((unsigned short*) (bytestream + 6));
    tmp.dwBytesInRes  = *((unsigned long*) (bytestream + 8));
    tmp.dwImageOffset = *((unsigned long*) (bytestream + 12));

    return tmp;
}

ico_t IcoRead(_In_ const wchar_t* const restrict filename) {
    static ico_t         tmp    = { 0 };
    size_t               imsize = 0;
    const uint8_t* const buffer = Open(filename, &imsize);

    if (!buffer) {
        // handle it, no need for error reporting, Open() will take care of that!
    }

    tmp.icBuffer = buffer;
}

bool IcoWrite(_In_ const ico_t* const restrict image) { }

void IcoInfo(_In_ const ico_t* const restrict image) { }
