#ifndef __UTILITIES_HPP
    #error this header is not meant to be used outside of the utilities namespace
#endif

#pragma once
#include <concepts>

// std::complex<>'s real() and imag() methods return a const reference even when the object is non const
// and it uses a 2 member array as the internal storage, so to update individual elements we need to expose the array and manually subscript into it
// opting for a handrolled complex alternative, fucking STL heh???
template<typename _Ty> requires std::is_arithmetic_v<_Ty> class complex final { // doesn't provide the arithmetic functionalities like std::complex<> though
    private:
        _Ty __x;
        _Ty __y;

    public:
        constexpr complex() noexcept : __x {}, __y {} { }

        constexpr explicit inline complex(const _Ty& v) noexcept : __x { v }, __y { v } { }

        constexpr inline complex(const _Ty& x, const _Ty& y) noexcept : __x { x }, __y { y } { }

        constexpr complex(const complex&) noexcept            = default;
        constexpr complex(complex&&) noexcept                 = default;
        constexpr complex& operator=(const complex&) noexcept = default;
        constexpr complex& operator=(complex&&) noexcept      = default;
        constexpr ~complex() noexcept                         = default;

        constexpr inline _Ty& __attribute__((__always_inline__)) x() noexcept { return __x; }

        constexpr inline _Ty __attribute__((__always_inline__)) x() const noexcept { return __x; }

        constexpr inline _Ty& __attribute__((__always_inline__)) y() noexcept { return __y; }

        constexpr inline _Ty __attribute__((__always_inline__)) y() const noexcept { return __y; }
};
