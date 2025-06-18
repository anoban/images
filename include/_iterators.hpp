#pragma once

// clang-format off
#include <internal.hpp>
// clang-format on

#include <cassert>
#include <iterator> // std::random_access_iterator_tag

template<typename _Ty> class random_access_iterator final { // unchecked random access iterator
        // if invalid memory access happens, the OS may raise an access violation exception, the iterator won't do anything about this in release mode
        // in debug mode, certain preventative asserts may fail, indicating where things went wrong

    public:
        using value_type        = _Ty;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;
        using difference_type   = signed long long;
        using size_type         = unsigned long long;
        using iterator_category = std::random_access_iterator_tag;

        // clang-format off
#ifndef __TEST__    // for testing purposes make the data members public!
    private:
#endif
        // clang-format on

        // using an unqualified pointer here will raise errors with const iterables!
        pointer   rsrc {};   // pointer to the iterable resource
        size_type length {}; // number of elements in the iterable
        size_type offset {}; // position in the iterable currently referenced by the iterator

        constexpr void __stdcall cleanup() noexcept {
            rsrc   = nullptr;
            length = offset = 0;
        }

        constexpr void __stdcall cleanup(_Inout_ random_access_iterator& other) const noexcept {
            other.rsrc   = nullptr;
            other.length = other.offset = 0;
        }

    public: // NOLINT(readability-redundant-access-specifiers)
        constexpr random_access_iterator() noexcept = default;

        constexpr random_access_iterator(_In_ _Ty* const _res, _In_ const size_type& _len) noexcept : rsrc(_res), length(_len), offset() {
            assert(_res);
            assert(_len);
        }

        constexpr random_access_iterator(_In_ _Ty* const _res, _In_ const size_type& _len, _In_ const size_type& _pos) noexcept :
            rsrc(_res), length(_len), offset(_pos) {
            assert(_res);
            assert(_len);
            assert(_len >= _pos);
        }

        constexpr random_access_iterator(_In_ const random_access_iterator& other) noexcept :
            rsrc(other.rsrc), length(other.length), offset(other.offset) { }

        constexpr random_access_iterator(_In_ random_access_iterator&& other) noexcept :
            rsrc(other.rsrc), length(other.length), offset(other.offset) {
            cleanup(other); // cleanup the stolen from resource
        }

        constexpr random_access_iterator& operator=(_In_ const random_access_iterator& other) noexcept {
            if (this == &other) return *this;
            rsrc   = other.rsrc;
            length = other.length;
            offset = other.offset;
            return *this;
        }

        constexpr random_access_iterator& operator=(_In_ random_access_iterator&& other) noexcept {
            if (this == &other) return *this;
            rsrc   = other.rsrc;
            length = other.length;
            offset = other.offset;
            cleanup(other);
            return *this;
        }

        ~random_access_iterator() noexcept { cleanup(); }

        constexpr reference __stdcall operator*() noexcept { return rsrc[offset]; }

        constexpr const_reference __stdcall operator*() const noexcept { return rsrc[offset]; }

        constexpr pointer _unwrapped() noexcept { return rsrc; }

        constexpr const_pointer _unwrapped() const noexcept { return rsrc; }

        constexpr void reset() noexcept { offset = 0; }

        constexpr random_access_iterator& __stdcall operator++() noexcept {
            offset++;
            assert(offset <= length);
            return *this;
        }

        constexpr random_access_iterator __stdcall operator++(int) noexcept {
            offset++;
            assert(offset <= length);
            return { rsrc, length, offset - 1 };
        }

        constexpr random_access_iterator& __stdcall operator--() noexcept {
            offset--;
            assert(offset <= length); // assert(_offset >= 0); won't help because _offset is unsigned so instead, check for wraparounds
            return *this;
        }

        constexpr random_access_iterator __stdcall operator--(int) noexcept {
            offset--;
            assert(offset <= length);
            return { rsrc, length, offset + 1 };
        }

        constexpr bool __stdcall operator==(_In_ const random_access_iterator& other) const noexcept {
            return rsrc == other.rsrc && offset == other.offset;
        }

        constexpr bool __stdcall operator!=(_In_ const random_access_iterator& other) const noexcept {
            return rsrc != other.rsrc || offset != other.offset;
        }

        constexpr bool __stdcall operator<(_In_ const random_access_iterator& other) const noexcept {
            return rsrc == other.rsrc && offset < other.offset;
        }

        constexpr bool __stdcall operator<=(_In_ const random_access_iterator& other) const noexcept {
            return rsrc == other.rsrc && offset <= other.offset;
        }

        constexpr bool __stdcall operator>(_In_ const random_access_iterator& other) const noexcept {
            return rsrc == other.rsrc && offset > other.offset;
        }

        constexpr bool __stdcall operator>=(_In_ const random_access_iterator& other) const noexcept {
            return rsrc == other.rsrc && offset >= other.offset;
        }

        template<typename _TyInt> // NOLINTNEXTLINE(modernize-use-constraints)
        constexpr typename std::enable_if<std::is_integral_v<_TyInt>, random_access_iterator>::type operator+(
            _In_ const _TyInt& stride
        ) const noexcept {
            assert(length >= offset + stride);
            return { rsrc, length, offset + stride };
        }

        template<typename _TyInt> // NOLINTNEXTLINE(modernize-use-constraints)
        constexpr typename std::enable_if<std::is_integral<_TyInt>::value, random_access_iterator>::type operator-(
            _In_ const _TyInt& stride
        ) const noexcept {
            assert(length >= offset - stride);
            return { rsrc, length, offset - stride };
        }

        constexpr difference_type operator+(_In_ const random_access_iterator& other) const noexcept {
            assert(rsrc == other.rsrc && length == other.length);
            assert(offset + other.offset <= length);
            return offset + other.offset;
        }

        constexpr difference_type operator-(_In_ const random_access_iterator& other) const noexcept {
            assert(rsrc == other.rsrc && length == other.length);
            assert(offset + other.offset <= length);
            return offset - other.offset;
        }
};

#undef __INTERNAL
