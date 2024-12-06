#pragma once
#define __INTERNAL
#if !defined(__ITERATORS) && !defined(__INTERNAL) && !defined(__TEST__)
    #error DO NOT DIRECTLY INCLUDE HEADERS PREFIXED WITH AN UNDERSCORE IN SOURCE FILES, USE THE UNPREFIXED VARIANTS WITHOUT THE .HPP EXTENSION.
#endif

#include <cassert>
#include <iterator> // std::random_access_iterator_tag

template<typename _Ty> class random_access_iterator { // unchecked random access iterator
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
        pointer   _rsrc {};   // pointer to the iterable resource
        size_type _length {}; // number of elements in the iterable
        size_type _offset {}; // position in the iterable currently referenced by the iterator

        constexpr void __stdcall cleanup() noexcept {
            _rsrc   = nullptr;
            _length = _offset = 0;
        }

        constexpr void __stdcall cleanup(_Inout_ random_access_iterator& other) const noexcept {
            other._rsrc   = nullptr;
            other._length = other._offset = 0;
        }

    public: // NOLINT(readability-redundant-access-specifiers)
        constexpr random_access_iterator() noexcept = default;

        constexpr random_access_iterator(_In_ _Ty* const _res, _In_ const size_type& _len) noexcept :
            _rsrc(_res), _length(_len), _offset() {
            assert(_res);
            assert(_len);
        }

        constexpr random_access_iterator(_In_ _Ty* const _res, _In_ const size_type& _len, _In_ const size_type& _pos) noexcept :
            _rsrc(_res), _length(_len), _offset(_pos) {
            assert(_res);
            assert(_len);
            assert(_len >= _pos);
        }

        constexpr random_access_iterator(_In_ const random_access_iterator& other) noexcept :
            _rsrc(other._rsrc), _length(other._length), _offset(other._offset) { }

        constexpr random_access_iterator(_In_ random_access_iterator&& other) noexcept :
            _rsrc(other._rsrc), _length(other._length), _offset(other._offset) {
            cleanup(other); // cleanup the stolen from resource
        }

        constexpr random_access_iterator& operator=(_In_ const random_access_iterator& other) noexcept {
            if (this == &other) return *this;
            _rsrc   = other._rsrc;
            _length = other._length;
            _offset = other._offset;
            return *this;
        }

        constexpr random_access_iterator& operator=(_In_ random_access_iterator&& other) noexcept {
            if (this == &other) return *this;
            _rsrc   = other._rsrc;
            _length = other._length;
            _offset = other._offset;
            cleanup(other);
            return *this;
        }

        ~random_access_iterator() noexcept { cleanup(); }

        constexpr reference __stdcall               operator*() noexcept { return _rsrc[_offset]; }

        constexpr const_reference __stdcall         operator*() const noexcept { return _rsrc[_offset]; }

        constexpr pointer                           _unwrapped() noexcept { return _rsrc; }

        constexpr const_pointer                     _unwrapped() const noexcept { return _rsrc; }

        constexpr void                              reset() noexcept { _offset = 0; }

        constexpr random_access_iterator& __stdcall operator++() noexcept {
            _offset++;
            assert(_offset <= _length);
            return *this;
        }

        constexpr random_access_iterator __stdcall operator++(int) noexcept {
            _offset++;
            assert(_offset <= _length);
            return { _rsrc, _length, _offset - 1 };
        }

        constexpr random_access_iterator& __stdcall operator--() noexcept {
            _offset--;
            assert(_offset <= _length); // assert(_offset >= 0); won't help because _offset is unsigned so instead, check for wraparounds
            return *this;
        }

        constexpr random_access_iterator __stdcall operator--(int) noexcept {
            _offset--;
            assert(_offset <= _length);
            return { _rsrc, _length, _offset + 1 };
        }

        constexpr bool __stdcall operator==(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc == other._rsrc && _offset == other._offset;
        }

        constexpr bool __stdcall operator!=(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc != other._rsrc || _offset != other._offset;
        }

        constexpr bool __stdcall operator<(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc == other._rsrc && _offset < other._offset;
        }

        constexpr bool __stdcall operator<=(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc == other._rsrc && _offset <= other._offset;
        }

        constexpr bool __stdcall operator>(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc == other._rsrc && _offset > other._offset;
        }

        constexpr bool __stdcall operator>=(_In_ const random_access_iterator& other) const noexcept {
            return _rsrc == other._rsrc && _offset >= other._offset;
        }

        template<typename _TyInt>
        constexpr typename std::enable_if<std::is_integral_v<_TyInt>, random_access_iterator>::type operator+(_In_ const _TyInt& stride
        ) const noexcept {
            assert(_length >= _offset + stride);
            return { _rsrc, _length, _offset + stride };
        }

        template<typename _TyInt>
        constexpr typename std::enable_if<std::is_integral<_TyInt>::value, random_access_iterator>::type operator-(_In_ const _TyInt& stride
        ) const noexcept {
            assert(_length >= _offset - stride);
            return { _rsrc, _length, _offset - stride };
        }

        constexpr difference_type operator+(_In_ const random_access_iterator& other) const noexcept {
            assert(_rsrc == other._rsrc && _length == other._length);
            assert(_offset + other._offset <= _length);
            return _offset + other._offset;
        }

        constexpr difference_type operator-(_In_ const random_access_iterator& other) const noexcept {
            assert(_rsrc == other._rsrc && _length == other._length);
            assert(_offset + other._offset <= _length);
            return _offset - other._offset;
        }
};

#undef __INTERNAL
