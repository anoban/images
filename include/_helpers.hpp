#pragma once

// clang-format off
#include <_internal.hpp>
#include <_compiler.hpp>
#include <_wingdi.hpp>
// clang-format on

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <iostream>
#include <limits>
#include <type_traits>

#include <endian.h>

#if __BYTE_ORDER != __LITTLE_ENDIAN
    #error This codebase assumes little endian hardware and will not run correctly on big endian hardware!
#endif

#include <immintrin.h> // AVX
#include <xmmintrin.h> // SSE

#if defined(DEBUG) || defined(_DEBUG)
    #define dbgputs(...)   ::fputs(__VA_ARGS__, stderr)
    #define dbgprintf(...) ::fprintf(stderr, __VA_ARGS__);
#else
    #define dbgputs(...)
    #define dbgprintf(...)
#endif

// clang-format off
#ifdef __TEST__
    #define __TEST_ONLY(access_specifier) access_specifier:
#else
    #define __TEST_ONLY(access_specifier)
#endif
// clang-format on

// to express our intention clearly, isntead of (ptr + n) ing all the time, if the passed ptr is NULL, return NULL
// the function receiving this ptr should null check it, as this macro itself cannot help with null derefs
#define if_not_null_offsetby(ubyteptr, offset)           (ubyteptr ? (ubyteptr + offset) : nullptr)
// this will help avoid some segfaults, the cast is there just to make the compilers shut up
#define if_not_null_offsetby_and_deref(ubyteptr, offset) (ubyteptr ? *(ubyteptr + offset) : static_cast<unsigned char>(0))

// RGB combinations of colours
enum class RGB : unsigned char { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE };

// mechanism to be used in converting the pixels to black and white
enum class BW : unsigned char { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY };

enum class ANGLES : unsigned short { NINETY = 90, ONEEIGHTY = 180, TWOSEVENTY = 270 };

// for the sake of convenience
static constexpr inline bool __attribute__((__always_inline__)) operator==(const RGBQUAD & left, const RGBQUAD & right) noexcept {
    return left.rgbBlue == right.rgbBlue && left.rgbGreen == right.rgbGreen && left.rgbRed == right.rgbRed;
}

static constexpr inline bool __attribute__((__always_inline__)) operator!=(const RGBQUAD & left, const RGBQUAD & right) noexcept {
    return left.rgbBlue != right.rgbBlue || left.rgbGreen != right.rgbGreen || left.rgbRed != right.rgbRed;
}

namespace internal {

    template<
        typename _TyFrom, // only for scoped enum types, plain C style enums support implicit type conversions so this isn't necessary
        typename _TyTo =  // the underlying type
        typename std::enable_if<
            std::is_enum<_TyFrom>::value, // std::is_scoped_enum type trait requires C++23
            typename std::underlying_type<_TyFrom>::type>::type>
    static constexpr inline _TyTo __attribute__((__always_inline__)) to_underlying(const _TyFrom& enumeration) noexcept {
        return static_cast<_TyTo>(enumeration);
    }

    template<typename _TyCandidate, typename _Ty> static constexpr inline
        typename std::enable_if<std::is_integral<_TyCandidate>::value && std::is_integral<_Ty>::value, bool>::type
        __attribute__((__always_inline__))
        is_in(const _TyCandidate& candidate, const _Ty& last) noexcept {
        return candidate == last;
    }

    template<typename _TyCandidate, typename _TyFirst, typename... _TyList> static constexpr inline
        typename std::enable_if<std::is_integral<_TyCandidate>::value && std::is_integral<_TyFirst>::value, bool>::type
        __attribute__((__always_inline__))
        is_in(const _TyCandidate& candidate, const _TyFirst& first, const _TyList&... list) noexcept {
        return candidate == first || internal::is_in(candidate, list...);
    }

    template<typename _TyValue, typename _TyLow, typename _TyHigh> static inline constexpr typename std::enable_if<
        std::is_arithmetic<_TyValue>::value && std::is_arithmetic<_TyLow>::value && std::is_arithmetic<_TyHigh>::value,
        bool>::type __attribute__((__always_inline__))
    is_within_inclusive_range(const _TyValue& value, const _TyLow& low, const _TyHigh& high) noexcept {
        return (value >= low) && (value <= high);
    }

    template<typename _TyTo> static inline _TyTo
        __attribute__((__always_inline__)) constexpr safe_deref(const unsigned char* const ptr, const long long& offset = 0) noexcept {
        if (!ptr) return static_cast<_TyTo>(0);
        return *reinterpret_cast<typename std::add_pointer<typename std::add_const<_TyTo>::type>::type>(ptr + offset);
    }

    static inline const unsigned char* __attribute__((__always_inline__)) safe_offset(
        const unsigned char* const ptr, const long long& offset
    ) noexcept {
        if (!ptr) return nullptr;
        return ptr + offset;
    }

    namespace ascii {

        // isalpha() from <cctype> is not constexpr :(
        [[nodiscard]] static constexpr inline bool __attribute__((__always_inline__)) is_alphabet(const char& character) noexcept {
            return (character >= 0x41 && character <= 0x5A) /* A - Z */ || (character >= 0x61 && character <= 0x7A); /* a - z */
        }

        [[nodiscard]] static constexpr inline bool __attribute__((__always_inline__)) is_alphabet_array(
            const char* const array, const unsigned long long length
        ) noexcept {
            bool result { true };
            for (unsigned i = 0; i < length; ++i) result &= ascii::is_alphabet(array[i]);
            return result;
        }

        // isupper() from <cctype> is not constexpr
        [[nodiscard]] static constexpr inline bool __attribute__((__always_inline__)) is_uppercase(const char& character) noexcept {
            return character >= 0x41 && character <= 0x5A; // A - 0x41 and Z - 0x5A
        }

        // islower() from <cctype> is not constexpr
        [[nodiscard]] static constexpr inline bool __attribute__((__always_inline__)) is_lowercase(const char& character) noexcept {
            return character >= 0x61 && character <= 0x7A; // a - 0x61 and z - 0x7A
        }

    } // namespace ascii

    namespace rgb {

        namespace transformers {

            // each colour value in the pixel is updated to their arithmetic average
            static constexpr inline void __attribute__((__always_inline__)) average(RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>((static_cast<long double>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.0L);
            }

            static constexpr inline void __attribute__((__always_inline__)) weighted_average(RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>(pixel.rgbBlue * 0.299L + pixel.rgbGreen * 0.587L + pixel.rgbRed * 0.114L);
            }

            static constexpr inline void __attribute__((__always_inline__)) luminosity(RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>(pixel.rgbBlue * 0.2126L + pixel.rgbGreen * 0.7152L + pixel.rgbRed * 0.0722L);
            }

            // every colour value gets scaled down to 0 or scaled up to 255 depending on the average value of colours in a pixel
            static constexpr inline void __attribute__((__always_inline__)) binary(RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    (static_cast<double>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.0 >= 128.0 ? 255 : 0;
            }

            // each colour value gets scaled down to 0 or scaled up to 255 depending on the corresponding value
            static constexpr inline void __attribute__((__always_inline__)) negative(RGBQUAD& pixel) noexcept {
                pixel.rgbBlue  = pixel.rgbBlue >= 128 ? 255 : 0;
                pixel.rgbGreen = pixel.rgbGreen >= 128 ? 255 : 0;
                pixel.rgbRed   = pixel.rgbRed >= 128 ? 255 : 0;
            }

        } // namespace transformers

        namespace removers { // this is really verbose but makes mutating the pixel buffers possible with a single std::for_each call

            template<RGB colour> struct zero;

            template<> struct zero<RGB::RED> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbRed = 0;
                    }
            };

            template<> struct zero<RGB::GREEN> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbGreen = 0;
                    }
            };

            template<> struct zero<RGB::BLUE> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbBlue = 0;
                    }
            };

            template<> struct zero<RGB::REDGREEN> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbRed = pixel.rgbGreen = 0;
                    }
            };

            template<> struct zero<RGB::GREENBLUE> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbGreen = pixel.rgbBlue = 0;
                    }
            };

            template<> struct zero<RGB::REDBLUE> final {
                    constexpr inline void __attribute__((__always_inline__)) operator()(RGBQUAD & pixel) const noexcept {
                        pixel.rgbRed = pixel.rgbBlue = 0;
                    }
            };

        } // namespace removers

    } // namespace rgb

    namespace crc {
        // for implementation details, refer https://lxp32.github.io/docs/a-simple-example-crc32-calculation/
        // IEEE (0xEDB88320) is by far the most common CRC-32 polynomial, used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...

        // computed and stored using the IEEE variant of the polynomial
        static constexpr std::array<unsigned, 256> IEEE_LOOKUPTABLE {
            { 0,          1996959894, 3993919788, 2567524794, 124634137,  1886057615, 3915621685, 2657392035, 249268274,  2044508324,
             3772115230, 2547177864, 162941995,  2125561021, 3887607047, 2428444049, 498536548,  1789927666, 4089016648, 2227061214,
             450548861,  1843258603, 4107580753, 2211677639, 325883990,  1684777152, 4251122042, 2321926636, 335633487,  1661365465,
             4195302755, 2366115317, 997073096,  1281953886, 3579855332, 2724688242, 1006888145, 1258607687, 3524101629, 2768942443,
             901097722,  1119000684, 3686517206, 2898065728, 853044451,  1172266101, 3705015759, 2882616665, 651767980,  1373503546,
             3369554304, 3218104598, 565507253,  1454621731, 3485111705, 3099436303, 671266974,  1594198024, 3322730930, 2970347812,
             795835527,  1483230225, 3244367275, 3060149565, 1994146192, 31158534,   2563907772, 4023717930, 1907459465, 112637215,
             2680153253, 3904427059, 2013776290, 251722036,  2517215374, 3775830040, 2137656763, 141376813,  2439277719, 3865271297,
             1802195444, 476864866,  2238001368, 4066508878, 1812370925, 453092731,  2181625025, 4111451223, 1706088902, 314042704,
             2344532202, 4240017532, 1658658271, 366619977,  2362670323, 4224994405, 1303535960, 984961486,  2747007092, 3569037538,
             1256170817, 1037604311, 2765210733, 3554079995, 1131014506, 879679996,  2909243462, 3663771856, 1141124467, 855842277,
             2852801631, 3708648649, 1342533948, 654459306,  3188396048, 3373015174, 1466479909, 544179635,  3110523913, 3462522015,
             1591671054, 702138776,  2966460450, 3352799412, 1504918807, 783551873,  3082640443, 3233442989, 3988292384, 2596254646,
             62317068,   1957810842, 3939845945, 2647816111, 81470997,   1943803523, 3814918930, 2489596804, 225274430,  2053790376,
             3826175755, 2466906013, 167816743,  2097651377, 4027552580, 2265490386, 503444072,  1762050814, 4150417245, 2154129355,
             426522225,  1852507879, 4275313526, 2312317920, 282753626,  1742555852, 4189708143, 2394877945, 397917763,  1622183637,
             3604390888, 2714866558, 953729732,  1340076626, 3518719985, 2797360999, 1068828381, 1219638859, 3624741850, 2936675148,
             906185462,  1090812512, 3747672003, 2825379669, 829329135,  1181335161, 3412177804, 3160834842, 628085408,  1382605366,
             3423369109, 3138078467, 570562233,  1426400815, 3317316542, 2998733608, 733239954,  1555261956, 3268935591, 3050360625,
             752459403,  1541320221, 2607071920, 3965973030, 1969922972, 40735498,   2617837225, 3943577151, 1913087877, 83908371,
             2512341634, 3803740692, 2075208622, 213261112,  2463272603, 3855990285, 2094854071, 198958881,  2262029012, 4057260610,
             1759359992, 534414190,  2176718541, 4139329115, 1873836001, 414664567,  2282248934, 4279200368, 1711684554, 285281116,
             2405801727, 4167216745, 1634467795, 376229701,  2685067896, 3608007406, 1308918612, 956543938,  2808555105, 3495958263,
             1231636301, 1047427035, 2932959818, 3654703836, 1088359270, 936918000,  2847714899, 3736837829, 1202900863, 817233897,
             3183342108, 3401237130, 1404277552, 615818150,  3134207493, 3453421203, 1423857449, 601450431,  3009837614, 3294710456,
             1567103746, 711928724,  3020668471, 3272380065, 1510334235, 755167117 }
        };

        // works great, tested and produces the same results as Python's binascii module's crc32() function
        [[nodiscard]] static constexpr inline unsigned __attribute__((__always_inline__)) calculate(
            const unsigned char* const bytestream, const unsigned long& length
        ) noexcept {
            if (!bytestream) {
                ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
                return 0;
            }
            unsigned checksum { 0xFFFFFFFF };
            for (size_t i = 0; i < length; ++i) checksum = (checksum >> 8) ^ IEEE_LOOKUPTABLE.at((bytestream[i] ^ checksum) & 0xFF);
            return ~checksum;
        }

    } // namespace crc

    namespace endian {

        [[maybe_unused]] static constexpr inline unsigned short __attribute__((__always_inline__)) u16_from_be_bytes(
            const unsigned char* const bytestream
        ) noexcept {
            static_assert(sizeof(unsigned short) == 2);
            if (!bytestream) {
                ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
                return 0;
            }
            return static_cast<unsigned short>(bytestream[0]) << 8 | bytestream[1];
        }

        // WARNING :: WITH LLVM, DO NOT PASS BUFFERS SHORTER THAN 8 BYTES IN LENGTH
        [[maybe_unused]] static inline unsigned __attribute__((__always_inline__)) u32_from_be_bytes(
            const unsigned char* const bytestream
        ) noexcept {
            static_assert(sizeof(unsigned) == 4);
            if (!bytestream) {
                ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
                return 0;
            }

#ifdef __llvm__ // LLVM defines __m64 as typedef __attribute__((__vector_size__(1 * sizeof(long long)))) long long __m64
            static constexpr __m64 mask_pi8 { 0x0405060700010203LL };
#elif defined(__GNUG__) // in GCC __m64 is typedef __attribute((vector_size(8))) int __m64
            // g++ bitches about a narrowing conversion here, "from long long int to int" but __m64 is in fact an 8 byte aligned vector BUT also an int???
            static const __m64 mask_pi8 {
                ::_mm_set_pi64x(0x0405060700010203LL) // ::_mm_set_pi64x() just does a simple type cast to __m64
            };
            // cannot constexpr this because ::_mm_set_pi64x() is not constexpr
#endif
            // move the first four bytes to the last four byte slot
            // even though only the first 4 bytes matter to this function, when reading in the stream of bytes as __m64, it'll dereference a sequence of 8 contiguous bytes

            return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];
        }

        [[maybe_unused]] static inline unsigned long long __attribute__((__always_inline__)) u64_from_be_bytes(
            const unsigned char* const bytestream
        ) noexcept {
            static_assert(sizeof(unsigned long long) == 8);
            if (!bytestream) {
                ::fprintf(stderr, "Error in %s, received an empty buffer\n", __PRETTY_FUNCTION__);
                return 0;
            }

#ifdef __llvm__
            static constexpr __m64 mask_pi8 { 0x01020304050607LL };
#elif defined(__GNUG__)
            static const __m64 mask_pi8 { ::_mm_set_pi64x(0x01020304050607LL) };
#endif
            return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];
        }

    } // namespace endian

    // std::complex<>'s real() and imag() methods return a const reference even when the object is non const
    // and it uses a 2 member array as the internal storage, so to update individual elements we need to expose the array and manually subscript into it
    // opting for a handrolled complex alternative, fucking STL heh???
    template<typename _Ty> requires std::is_arithmetic_v<_Ty>
    class complex final { // doesn't provide the arithmetic functionalities like std::complex<> though
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

} // namespace internal

#undef __INTERNAL
