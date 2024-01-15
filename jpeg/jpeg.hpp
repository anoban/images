#pragma once
#ifndef __JPEG_H_
    #define __JPEG_H_

    #include <concepts>
    #include <cstdint>

    #ifdef _WIN32
        #define _AMD64_     // architecture
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
    #endif
    #include <windef.h>     // for Win32 typedefs
    #include <wingdi.h>     // let's use Win32's RGBTRIPLE and RGBQUAD structs instead of redundant redefinitions

namespace jpeg {

    namespace constants { } // namespace constants

    // JPEG uses YCbCr format for storing pixels.
    template<typename T> requires std::integral<T> || std::floating_point<T> struct YCbCr {
            T Y {};  // Y - luminance or brightness of the pixel
            T Cb {}; // Cb - Blue
            T Cr {}; // Cr - Red
                     // Y component is called Luma component and the Cr and Cb components are collectively called Chroma components.
    };

    template<typename T /* input type */, typename OT /* output type */>
    requires std::is_same<RGBTRIPLE, T>::value || std::is_same<RGBQUAD, T>::value
    inline constexpr YCbCr<OT> to_YCbCr(_In_ const T& pixel) noexcept {
        // Y = (0.299 x R) + (0.587 x G) + (0.114 x B)
        // Cb = (-0.1687 x R) - (0.3313 x G) + (0.5 x B) + 128
        // Cr = (0.5 x R)  - (0.4187 x G) - (0.0813 x B) + 128

        constexpr auto Y { (0.299L * pixel.rgbRed) + (0.587L * pixel.rgbGreen) + (0.114L * pixel.rgbBlue) };
        constexpr auto Cb { (-0.1687L * pixel.rgbRed) - (0.3313L * pixel.rgbGreen) + (0.5L * pixel.rgbBlue) + 128.0L };
        constexpr auto Cr { (0.5L * pixel.rgbRed) - (0.4187L * pixel.rgbGreen) - (0.0813L * pixel.rgbBlue) + 128.0L };

        return YCbCr<OT> {
            {static_cast<OT>(Y), static_cast<OT>(Cb), static_cast<OT>(Cr)}
        };
    }

} // namespace jpeg

#endif //__JPEG_H_