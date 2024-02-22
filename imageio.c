#define _AMD64_ // architecture
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <heapapi.h>
#include <stdint.h>
#include <stdio.h>
#include <windef.h>

// a generic file reading routine, that reads in an existing binary file and returns the buffer. (NULL in case of a failure)
// returned memory needs to be freed using HeapFree()! NOT UCRT's free()
uint8_t* open(_In_ const wchar_t* const restrict file_name, _Inout_ size_t* const restrict size) {
    uint8_t*       buffer  = NULL;
    DWORD          nbytes  = 0UL;
    LARGE_INTEGER  liFsize = { .QuadPart = 0LLU };
    const HANDLE64 hFile   = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        goto INVALID_HANDLE_ERR;
    }

    // process's default heap, non serializeable
    const HANDLE64 hProcHeap = GetProcessHeap();
    if (!hProcHeap) {
        fwprintf_s(stderr, L"Error %lu in GetProcessHeap\n", GetLastError());
        goto GET_FILESIZE_ERR;
    }

    if (!GetFileSizeEx(hFile, &liFsize)) {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", GetLastError());
        goto GET_FILESIZE_ERR;
    }

    if (!(buffer = HeapAlloc(hProcHeap, 0UL, liFsize.QuadPart))) {
        fwprintf_s(stderr, L"Error %lu in HeapAlloc\n", GetLastError());
        goto GET_FILESIZE_ERR;
    }

    if (!ReadFile(hFile, buffer, liFsize.QuadPart, &nbytes, NULL)) {
        fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
        goto READFILE_ERR;
    }

    CloseHandle(hFile);
    *size = liFsize.QuadPart;
    return buffer;

READFILE_ERR:
    HeapFree(hProcHeap, 0UL, buffer);
GET_FILESIZE_ERR:
    CloseHandle(hFile);
INVALID_HANDLE_ERR:
    *size = 0;
    return NULL;
}