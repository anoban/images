#pragma once
#define __INTERNAL
#if !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

// clang-format off
#define _AMD64_ // architecture
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
// clang-format on

#include <cassert>
#include <cstdio>
#include <new>

namespace internal {

    // a generic file reading routine, that reads in an existing binary file and returns the buffer, (nullptr in case of a failure)
    // returned memory needs to be freed (`delete[]` ed) by the caller
    static inline unsigned char* __cdecl open( // NOLINT(readability-redundant-inline-specifier)
        _In_ const wchar_t* const filename,
        _Inout_ unsigned long&    size
    ) noexcept {
        assert(filename); // ????

        unsigned char* buffer {};
        LARGE_INTEGER  fsize {
             { .LowPart = 0LLU, .HighPart = 0LLU }
        };
        const HANDLE64 file_handle { ::CreateFileW(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr) };

        if (file_handle == INVALID_HANDLE_VALUE) {
            ::fwprintf_s( // NOLINT(cppcoreguidelines-pro-type-vararg)
                stderr,
                L"Error %lu in CreateFileW (%s)\n",
                ::GetLastError(),
                filename
            );
            goto INVALID_HANDLE_ERROR;
        }

        if (!::GetFileSizeEx(file_handle, &fsize)) { // NOLINT(readability-implicit-bool-conversion)
            ::fwprintf_s(                            // NOLINT(cppcoreguidelines-pro-type-vararg)
                stderr,
                L"Error %lu in GetFileSizeEx (%s)\n",
                ::GetLastError(),
                filename
            );
            goto GETFILESIZEEX_ERROR;
        }

        buffer = new (std::nothrow) unsigned char[fsize.QuadPart];
        if (!buffer) { // NOLINT(readability-implicit-bool-conversion)
            ::fputws(L"memory allocation failed inside " __FUNCTIONW__ "\n", stderr);
            goto GETFILESIZEEX_ERROR;
        }

        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (!::ReadFile(file_handle, buffer, fsize.QuadPart, &size, nullptr)) {
            ::fwprintf_s(stderr, L"Error %lu in ReadFile (%s)\n", ::GetLastError(), filename); // NOLINT(cppcoreguidelines-pro-type-vararg)
            goto READFILE_ERROR;
        }

        // let's be optimistic here
        ::CloseHandle(file_handle);
        return buffer;

READFILE_ERROR:
        delete[] buffer;
GETFILESIZEEX_ERROR:
        ::CloseHandle(file_handle);
INVALID_HANDLE_ERROR:
        size = 0;
        return nullptr;
    }

    // a file format agnostic write routine to serialize binary image files, if a file with the specified name exists on disk, it will be overwritten
    static inline bool __cdecl serialize( // NOLINT(readability-redundant-inline-specifier)
        _In_ const wchar_t* const                         filename,
        _In_reads_bytes_(size) const unsigned char* const buffer,
        _In_ const unsigned long                          size
    ) noexcept {
        assert(filename);          // too much??
        if (!buffer) return false; // fail if the buffer is a nullptr

        const HANDLE64 file_handle { ::CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) };
        unsigned long  nbytes {};

        if (file_handle == INVALID_HANDLE_VALUE) {
            ::fwprintf_s( // NOLINT(cppcoreguidelines-pro-type-vararg)
                stderr,
                L"Error %4lu in CreateFileW (%s)\n",
                ::GetLastError(),
                filename
            );
            goto PREMATURE_RETURN;
        }

        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (!::WriteFile(file_handle, buffer, size, &nbytes, nullptr)) {
            ::fwprintf_s( // NOLINT(cppcoreguidelines-pro-type-vararg)
                stderr,
                L"Error %4lu in WriteFile (%s)\n",
                ::GetLastError(),
                filename
            );
            goto PREMATURE_RETURN;
        }

        ::CloseHandle(file_handle);
        return true;

PREMATURE_RETURN:
        ::CloseHandle(file_handle);
        return false;
    }

} // namespace internal

#undef __INTERNAL
