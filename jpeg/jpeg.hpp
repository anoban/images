#pragma once
#ifndef __JPEG_H_
    #define __JPEG_H_

    #include <concepts>
    #include <cstdint>

namespace jpeg {

    #pragma pack(push, 1)
    struct RGBTRIPLE {
            uint8_t BLUE {};
            uint8_t GREEN {};
            uint8_t RED {};
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct RGBQUAD : public RGBTRIPLE {
            uint8_t RESERVED { 0xFF }; // must be 0, but seems to be 0xFF in most BMPs.
    };
    #pragma pack(pop)

    // JPEG usesYCbCr format for storing pixels.

    #pragma pack(push, 1)
    struct YCbCr {
            // Y - muminance or brightness of the pixel
            // Cb - Blue
            // Cr - Red
            // Y component is called Luma component and the Cr and Cb components are collectively called Chroma components.
    };
    #pragma pack(pop)

    template<typename T> requires std::is_base_of<RGBTRIPLE, T>::value constexpr YCbCr toYCbCr(_In_ const T& pixel) noexcept {
        T     tmp {};
        YCbCr pix { .};

        /*
        Y=0.299∗R+0.587∗G+0.114∗B

        Cb=−0.1687∗R−0.3313∗G+0.5∗B+128
        Cr=0.5∗R−0.4187∗G−0.0813∗B+128
        */
    }

} // namespace jpeg

#endif //__JPEG_H_