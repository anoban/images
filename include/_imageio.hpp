#pragma once

// clang-format off
#include <internal.hpp>
// clang-format on

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <new>

#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

namespace internal {

    // a generic file reading routine, that reads in an existing binary file and returns the buffer, (nullptr in case of a failure)
    // returned memory needs to be freed (`delete[]` ed) by the caller
    [[nodiscard]] static inline unsigned char* imopen(const char* const fpath, unsigned long& nreadbytes) {
        nreadbytes = 0;
        unsigned char* buffer {};
        struct stat    filestat {};
        long           nbytes {};

        const int fdesc { ::open(fpath, O_RDONLY) };
        if (fdesc == -1) { // if open() failed, the return value will be -1
            ::fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
            return nullptr;
        }

        if (::fstat(fdesc, &filestat)) { // if succeeds, 0 is returned, -1 if fails
            ::fprintf(stderr, "Call to fstat() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
            goto CLOSE_AND_RETURN;
        }

        if (!(buffer = new (std::nothrow) unsigned char[filestat.st_size])) { // caller is responsible for freeing this buffer
            ::fprintf(stderr, "Call to new() failed inside %s at line %d!\n", __FUNCTION__, __LINE__);
            goto CLOSE_AND_RETURN;
        }

        if ((nbytes = ::read(fdesc, buffer, filestat.st_size)) != -1) {
            nreadbytes = nbytes;
            assert(nbytes == filestat.st_size); // double checking
        } else {
            ::fprintf(stderr, "Call to read() failed inside %s at line %lu!; errno %d\n", __FUNCTION__, __LINE__, errno);
            delete buffer;
            buffer = nullptr;
        }
        // then, fall through the CLOSE_AND_RETURN label

CLOSE_AND_RETURN:
        // close() returns 0 on success and -1 on failure
        if (::close(fdesc)) ::fprintf(stderr, "Call to close() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        return buffer;
    }

    // a file format agnostic write routine to serialize binary image files, if a file with the specified name exists on disk, it will be overwritten
    [[nodiscard]]  static inline bool  imwrite( // NOLINT(readability-redundant-inline-specifier)
         const char* const                         filename,
         const unsigned char* const buffer,
         const unsigned long&                          size
    ) noexcept {
        assert(filename); // too much??
        if (!buffer) {
            ::fprintf(stderr, "Empty buffer passed to function %s at line %d\n", __FUNCTION__, __LINE__);
            return false; // fail if the buffer is a nullptr
        }

        bool          is_success {};                                // has every step succeeded???
        unsigned long nbytes {};                                    // number of bytes serialized to the disk
        const int     fdesc { ::open(filename, O_CREAT | O_RDWR) }; // open the file descriptor with create and write privileges

        if (fdesc == -1) {
            ::fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
            goto CLOSE_AND_RETURN;
        }

        nbytes = ::write(fdesc, buffer, size);
        if (nbytes == -1) {
            ::fprintf(stderr, "Call to write() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
            goto CLOSE_AND_RETURN;
        }

        // if the write was successful,
        assert(size == nbytes);
        is_success = true;
        // then, fall through the CLOSE_AND_RETURN label

CLOSE_AND_RETURN:
        ::close(fdesc);
        return is_success;
    }

} // namespace internal

#undef __INTERNAL
