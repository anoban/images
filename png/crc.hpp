#pragma once
#ifndef __CRC_H_
    #define __CRC_H_

    #include <array>
    #include <cstdint>
    #include <vector>

namespace crc {

    [[nodiscard, msvc::flatten, msvc::forceinline]] static inline const std::array<size_t, 256> constexpr lookuptable(void) noexcept {
        std::array<size_t, 256> table {};
        uint32_t                byte {}, crc {};

        for (uint32_t i = 0; i < 256; ++i) {
            byte = i;
            crc  = 0;
            for (size_t j = 0; j < 8; ++j) {
                uint32_t b   = (byte ^ crc) & 0x1;
                crc        >>= 1;
                if (b) crc = crc ^ 0xEDB88320;
                byte >>= 1;
            }
            table.at(i) = crc;
        }
        return table;
    }

    [[nodiscard, deprecated /* inefficient, use the alternative that leverages the lookup table */
    ]] static inline const uint32_t constexpr crc32(_In_ const std::vector<uint8_t>& stream) noexcept {
        uint32_t crc { 0xFFFFFFFF };
        uint8_t  tmp {};

        for (const auto& byte : stream) {
            tmp = byte;
            for (size_t j = 0; j < 8; ++j) {
                uint32_t b   = (tmp ^ crc) & 0x1;
                crc        >>= 1;
                if (b) crc = crc ^ 0xEDB88320;
                tmp >>= 1;
            }
        }

        return ~crc;
    }

} // namespace crc

#endif // !__CRC_H_