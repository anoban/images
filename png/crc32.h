#pragma once
#ifndef __CRC32_H_
    #define __CRC32_H_
    #include <stdbool.h>
    #include <stdint.h>

// IEEE is by far and away the most common CRC-32 polynomial.
// Used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
static const uint32_t IEEE                    = 0xEDB88320;

// Castagnoli's polynomial, used in iSCSI.
// Has better error detection characteristics than IEEE. https://dx.doi.org/10.1109/26.231911
static const uint32_t CASTAGNOLI              = 0x82F63B78;

// Koopman's polynomial.
// Also has better error detection characteristics than IEEE. https://dx.doi.org/10.1109/DSN.2002.1028931
static const uint32_t KOOPMAN                 = 0xEB31D82E;

static uint32_t       CRC32_LOOKUPTABLE[256]  = { 0 };

static bool           is_lkptable_initialized = false;

// for implementation details, refer https://lxp32.github.io/docs/a-simple-example-crc32-calculation/
static void __forceinline __stdcall populate(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t ch  = i;
        uint32_t crc = 0;
        for (size_t j = 0; j < 8; j++) {
            uint32_t b   = (ch ^ crc) & 1;
            crc        >>= 1;
            if (b) crc = crc ^ IEEE;
            ch >>= 1;
        }
        CRC32_LOOKUPTABLE[i] = crc;
    }
}

// carries dependancy on CRC32_LOOKUPTABLE
static __forceinline uint32_t __stdcall crc32checksum(
    _In_count_c_(length) const uint8_t* const restrict bytestream, _In_ const size_t length
) {
    if (!is_lkptable_initialized) {
        populate();
        is_lkptable_initialized = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) crc = (crc >> 8) ^ CRC32_LOOKUPTABLE[(bytestream[i] ^ crc) & 0xFF];
    return ~crc;
}

#endif