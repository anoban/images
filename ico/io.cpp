#define _AMD64_ // architecture
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN
#include <cstdint>
#include <cstdio>
#include <optional>
#include <vector>

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <windef.h>

#include <ico.hpp>

std::optional<std::vector<uint8_t>> ico::io::Open(_In_ const wchar_t* const filename) {
    std::vector<uint8_t> buffer {};
    DWORD                nbytes {};
    LARGE_INTEGER        liFsize { .QuadPart = 0LLU };
    const HANDLE64       hFile { ::CreateFileW(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr) };

    if (hFile == INVALID_HANDLE_VALUE) {
        ::fwprintf_s(stderr, L"Error %lu in CreateFileW\n", ::GetLastError());
        goto INVALID_HANDLE_ERR;
    }

    if (!::GetFileSizeEx(hFile, &liFsize)) {
        ::fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", ::GetLastError());
        goto GET_FILESIZE_ERR;
    }

    buffer.resize(liFsize.QuadPart);

    if (!::ReadFile(hFile, buffer.data(), liFsize.QuadPart, &nbytes, nullptr)) {
        ::fwprintf_s(stderr, L"Error %lu in ReadFile\n", ::GetLastError());
        goto GET_FILESIZE_ERR;
    }

    ::CloseHandle(hFile);
    return buffer;

GET_FILESIZE_ERR:
    ::CloseHandle(hFile);
INVALID_HANDLE_ERR:
    return std::nullopt;
}

bool ico::io::Serialize(_In_ const wchar_t* const filename, _In_ const std::vector<uint8_t>& buffer) noexcept {
    const HANDLE64 hFile { ::CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) };
    DWORD          nbytes {};

    if (hFile == INVALID_HANDLE_VALUE) {
        ::fwprintf_s(stderr, L"Error %4lu in CreateFileW\n", ::GetLastError());
        goto PREMATURE_RETURN;
    }

    if (!::WriteFile(hFile, buffer.data(), buffer.size() /* the element type is uint8_t, so */, &nbytes, nullptr)) {
        ::fwprintf_s(stderr, L"Error %4lu in WriteFile\n", ::GetLastError());
        goto PREMATURE_RETURN;
    }

    ::CloseHandle(hFile);
    return true;

PREMATURE_RETURN:
    ::CloseHandle(hFile);
    return false;
}
