#pragma once
#ifndef __ICO_HPP__
    #define __ICO_HPP__

    #include <optional>
    #include <vector>

namespace ico {

    namespace io {
        std::optional<std::vector<uint8_t>> Open(_In_ const wchar_t* const filename) noexcept;
        bool                                Serialize(_In_ const wchar_t* const filename, _In_ const std::vector<uint8_t>& buffer) noexcept;
    } // namespace io

    class ico {
        public:
            ico() = delete; // no default ctors
            inline ico(_In_ const wchar_t* const filename) noexcept;

        private:
    }; // class ico
} // namespace ico

#endif // !__ICO_HPP__
