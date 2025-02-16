#pragma once
#define __INTERNAL
#if !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#define NOMINMAX // NOMINMAX only works with <Windows.h>, when system headers are directly included without relying on <Windows.h>
// NOMINMAX offers no help as it seems only <Windows.h> has the #undef directives receptive to NOMINMAX
#ifdef min
    #undef min
#endif
#ifdef max
    #undef max
#endif

#include <array>
#include <cassert>
#include <iostream>
#include <type_traits>

// clang-format off
#define _AMD64_
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_MEAN
#include <intrin.h>
#include <windef.h>
#include <wingdi.h>
#include <WinSock2.h>
// clang-format on

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Ws2_32.lib")

#if defined(DEBUG) || defined(_DEBUG)
    #define dbgputws(...)     ::fputws(__VA_ARGS__, stderr)
    #define dbgwprintf_s(...) ::fwprintf_s(stderr, __VA_ARGS__);
#else
    #define dbgputws(...)
    #define dbgwprintf_s(...)
#endif

// RGB combinations of colours
enum class RGB_TAG : unsigned char { RED, GREEN, BLUE, REDGREEN, REDBLUE, GREENBLUE };

// mechanism to be used in converting the pixels to black and white
enum class BW_TRANSFORMATION : unsigned char { AVERAGE, WEIGHTED_AVERAGE, LUMINOSITY, BINARY };

enum class ANGLES : unsigned short { NINETY = 0x5A, ONEEIGHTY = 180, TWOSEVENTY = 270, THREESIXTY = 360 };

// for the sake of convenience
static constexpr bool __stdcall operator==(_In_ const RGBQUAD& left, _In_ const RGBQUAD& right) noexcept {
    return left.rgbBlue == right.rgbBlue && left.rgbGreen == right.rgbGreen && left.rgbRed == right.rgbRed;
}

static constexpr bool __stdcall operator!=(_In_ const RGBQUAD& left, _In_ const RGBQUAD& right) noexcept {
    return left.rgbBlue != right.rgbBlue || left.rgbGreen != right.rgbGreen || left.rgbRed != right.rgbRed;
}

namespace internal {

    template<
        typename _TyFrom, /* only for scoped enum types, plain C style enums support implicit type conversions so this isn't necessary */
        typename _TyTo =  /* the underlying type */
        typename std::enable_if<
            std::is_enum<_TyFrom>::value, /* std::is_scoped_enum type trait requires C++23 */
            typename std::underlying_type<_TyFrom>::type>::type>
    static constexpr _TyTo __stdcall to_underlying(_In_ const _TyFrom& enumeration) noexcept {
        return static_cast<_TyTo>(enumeration);
    }

    template<typename _TyCandidate, typename _Ty> // NOLINTNEXTLINE(modernize-use-constraints)
    static constexpr typename std::enable_if<std::is_integral<_TyCandidate>::value && std::is_integral<_Ty>::value, bool>::type is_in(
        _In_ const _TyCandidate& candidate, _In_ const _Ty& last
    ) noexcept {
        return candidate == last;
    }

    template<typename _TyCandidate, typename _TyFirst, typename... _TyList> // NOLINTNEXTLINE(modernize-use-constraints)
    static constexpr typename std::enable_if<std::is_integral<_TyCandidate>::value && std::is_integral<_TyFirst>::value, bool>::type is_in(
        _In_ const _TyCandidate& candidate, _In_ const _TyFirst& first, _In_ const _TyList&... list
    ) noexcept {
        return candidate == first || internal::is_in(candidate, list...);
    }

    namespace ascii {

        // isalpha() from <cctype> is not constexpr :(
        [[nodiscard]] static constexpr bool __stdcall is_alphabet(_In_ const char& character) noexcept {
            return (character >= 0x41 && character <= 0x5A) /* A - Z */ || (character >= 0x61 && character <= 0x7A); /* a - z */
        }

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)

        [[nodiscard]] static constexpr bool __stdcall is_alphabet_array(
            _In_ const char* const array, _In_ const unsigned long long length
        ) noexcept {
            bool result { true };
            for (unsigned i = 0; i < length; ++i)
                result &= ascii::is_alphabet(array[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return result;
        }

        // isupper() from <cctype> is not constexpr
        [[nodiscard]] static constexpr bool __stdcall is_uppercase(_In_ const char& character) noexcept {
            return character >= 0x41 && character <= 0x5A; /* A - 0x41 and Z - 0x5A */
        }

        // islower() from <cctype> is not constexpr
        [[nodiscard]] static constexpr bool __stdcall is_lowercase(_In_ const char& character) noexcept {
            return character >= 0x61 && character <= 0x7A; /* a - 0x61 and z - 0x7A */
        }

    } // namespace ascii

    namespace rgb {

        namespace transformers {

            // each colour value in the pixel is updated to their arithmetic average
            static constexpr void __stdcall average(_Inout_ RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>((static_cast<long double>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.0L);
            }

            static constexpr void __stdcall weighted_average(_Inout_ RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>(pixel.rgbBlue * 0.299L + pixel.rgbGreen * 0.587L + pixel.rgbRed * 0.114L);
            }

            static constexpr void __stdcall luminosity(_Inout_ RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    static_cast<unsigned char>(pixel.rgbBlue * 0.2126L + pixel.rgbGreen * 0.7152L + pixel.rgbRed * 0.0722L);
            }

            // every colour value gets scaled down to 0 or scaled up to 255 depending on the average value of colours in a pixel
            static constexpr void __stdcall binary(_Inout_ RGBQUAD& pixel) noexcept {
                pixel.rgbBlue = pixel.rgbGreen = pixel.rgbRed =
                    (static_cast<double>(pixel.rgbBlue) + pixel.rgbGreen + pixel.rgbRed) / 3.0 >= 128.0 ? 255Ui8 : 0Ui8;
            }

        } // namespace transformers

        // each colour value gets scaled down to 0 or scaled up to 255 depending on the corresponding value
        static constexpr void __stdcall negative(_Inout_ RGBQUAD& pixel) noexcept {
            pixel.rgbBlue  = pixel.rgbBlue >= 128 ? 255Ui8 : 0Ui8;
            pixel.rgbGreen = pixel.rgbGreen >= 128 ? 255Ui8 : 0Ui8;
            pixel.rgbRed   = pixel.rgbRed >= 128 ? 255Ui8 : 0Ui8;
        }

        namespace removers { // this is really verbose but makes mutating the pixel buffers possible with a single std::for_each call

            template<RGB_TAG colour> struct zero;

            template<> struct zero<RGB_TAG::RED> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbRed = 0; }
            };

            template<> struct zero<RGB_TAG::GREEN> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbGreen = 0; }
            };

            template<> struct zero<RGB_TAG::BLUE> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbBlue = 0; }
            };

            template<> struct zero<RGB_TAG::REDGREEN> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbRed = pixel.rgbGreen = 0; }
            };

            template<> struct zero<RGB_TAG::GREENBLUE> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbGreen = pixel.rgbBlue = 0; }
            };

            template<> struct zero<RGB_TAG::REDBLUE> final {
                    constexpr void __stdcall operator()(_Inout_ RGBQUAD& pixel) const noexcept { pixel.rgbRed = pixel.rgbBlue = 0; }
            };

        } // namespace removers

    } // namespace rgb

    namespace crc { // for implementation details, refer https://lxp32.github.io/docs/a-simple-example-crc32-calculation/

        enum class POLYNOMIALS : unsigned {
            // IEEE is by far the most common CRC-32 polynomial, used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
            IEEE       = 0xEDB88320,
            // Castagnoli's polynomial, used in iSCSI, has better error detection characteristics than IEEE https://dx.doi.org/10.1109/26.231911
            CASTAGNOLI = 0x82F63B78,
            // Koopman's polynomial, also has better error detection characteristics than IEEE https://dx.doi.org/10.1109/DSN.2002.1028931
            KOOPMAN    = 0xEB31D82E
        };

        // computed and stored using the IEEE variant of the polynomial
        static constexpr std::array<unsigned, 256> CRC32_LOOKUPTABLE_IEEE {
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

        // works great, tested and produces the same results as Python's binascii.crc32()
        [[nodiscard]] static constexpr unsigned __stdcall get(
            _In_count_(length) const unsigned char* const bytestream, _In_ const unsigned long long length
        ) noexcept {
            unsigned crc { 0xFFFFFFFF }; // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (size_t i = 0; i < length; ++i) crc = (crc >> 8) ^ CRC32_LOOKUPTABLE_IEEE.at((bytestream[i] ^ crc) & 0xFF);
            return ~crc;
        }

    } // namespace crc

    namespace endian {

#if !defined(__llvm__) && !defined(_MSC_FULL_VER)
    #error routines inside namespace endian liberally rely on LLVM and MSVC compiler intrinsics, hence probably won't compile with other compilers!
#endif

        [[maybe_unused, nodiscard]] static constexpr unsigned short __stdcall ushort_from_be_bytes(_In_reads_bytes_(2
        ) const unsigned char* const bytestream) noexcept {
            assert(bytestream);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return static_cast<unsigned short>(bytestream[0]) << 8 | bytestream[1];
        }

        // WARNING :: WITH LLVM, DO NOT PASS BUFFERS SHORTER THAN 8 BYTES IN LENGTH
        [[maybe_unused, nodiscard]] static unsigned long __stdcall ulong_from_be_bytes(_In_reads_bytes_(8)
                                                                                           const unsigned char* const bytestream) noexcept {
            assert(bytestream);
#if defined(__llvm__) && defined(__clang__)                         // LLVM defines __m64 as a vector of 1 long long
            static constexpr __m64 mask_pi8 { 0x0405060700010203 }; // move the first four bytes to the last four byte slot
            // even though only the first 4 bytes matter to this function, when reading in the stream of bytes as __m64, it'll dereference a sequence of 8 contiguous bytes

            return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];

#elif defined(_MSC_VER) && defined(_MSC_FULL_VER)

            static constexpr __m128i mask_epi8 {
                .m128i_u8 { 3, 2, 1, 0, /* we don't care about the rest */ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
            };

            const __m128i operand_epi8 {
                .m128i_u8 { bytestream[0],
                           bytestream[1],
                           bytestream[2],
                           bytestream[3], // followed by 12 filler zeroes
                            0, 0,
                           0, 0,
                           0, 0,
                           0, 0,
                           0, 0,
                           0, 0 }
            };

            // unlike LLVM, MSVC offers the SSSE3 intrinsic _mm_shuffle_pi8 only in x86 mode, in x64 mode we could only use the AVX1 intrinsic _mm_shuffle_epi8
            return ::_mm_shuffle_epi8(operand_epi8, mask_epi8).m128i_u32[0]; // MSVC defines __m128i as a union
#endif
        }

        [[maybe_unused, nodiscard]] static unsigned long long __stdcall ullong_from_be_bytes(_In_reads_bytes_(8
        ) const unsigned char* const bytestream) noexcept {
            assert(bytestream);
#if defined(__llvm__) && defined(__clang__)
            static constexpr __m64 mask_pi8 { 0x01020304050607 };
            return ::_mm_shuffle_pi8(*reinterpret_cast<const __m64*>(bytestream), mask_pi8)[0];

#elif defined(_MSC_VER) && defined(_MSC_FULL_VER)

            static constexpr __m128i mask_epi8 {
                .m128i_u8 { 7, 6, 5, 4, 3, 2, 1, 0, /* we don't care about the rest */ 8, 9, 10, 11, 12, 13, 14, 15 }
            };
            const __m128i operand_epi8 {
                .m128i_u8 { bytestream[0],
                           bytestream[1],
                           bytestream[2],
                           bytestream[3],
                           bytestream[4],
                           bytestream[5],
                           bytestream[6],
                           bytestream[7],
                           0, /* followed by 8 filler zeroes */
                            0, 0,
                           0, 0,
                           0, 0,
                           0 }
            };
            return ::_mm_shuffle_epi8(operand_epi8, mask_epi8).m128i_u64[0];
#endif
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

            constexpr explicit complex(const _Ty& v) noexcept : __x { v }, __y { v } { }

            constexpr complex(const _Ty& x, const _Ty& y) noexcept : __x { x }, __y { y } { }

            constexpr complex(const complex&) noexcept            = default;
            constexpr complex(complex&&) noexcept                 = default;
            constexpr complex& operator=(const complex&) noexcept = default;
            constexpr complex& operator=(complex&&) noexcept      = default;
            constexpr ~complex() noexcept                         = default;

            constexpr _Ty& x() noexcept { return __x; }

            constexpr _Ty x() const noexcept { return __x; }

            constexpr _Ty& y() noexcept { return __y; }

            constexpr _Ty y() const noexcept { return __y; }
    };

} // namespace internal

#undef __INTERNAL
