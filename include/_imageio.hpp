#pragma once

// clang-format off
#include <internal.hpp>
// clang-format on

#include <cassert>
#include <cstdio>
#include <new>

#include <sys/fcntl.h>
#include <sys/stat.h>

namespace internal {

    // a generic file reading routine, that reads in an existing binary file and returns the buffer, (nullptr in case of a failure)
    // returned memory needs to be freed (`delete[]` ed) by the caller
    static inline unsigned char*  open( // NOLINT(readability-redundant-inline-specifier)
         const char* const filename,
         unsigned long&    size
    ) noexcept {
        assert(filename); // ????
        unsigned char* buffer {};
        struct stat    fstats {};

        const int filedesc = ::open(filename, O_RDONLY);

        ::fstat(filedesc, &fstats);

        buffer = new (std::nothrow) unsigned char[fstats.st_size];
        if (!buffer) { // NOLINT(readability-implicit-bool-conversion)
            ::fputs("memory allocation failed inside " __PRETTY_FUNCTION__ "\n", stderr);
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
    static inline bool  serialize( // NOLINT(readability-redundant-inline-specifier)
         const char* const                         filename,
         const unsigned char* const buffer,
         const unsigned long                          size
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
