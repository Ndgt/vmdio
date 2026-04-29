#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace vmdio
{
    /**
     * @class VMDString
     * @brief Value type for fixed-length Shift_JIS string fields in VMD files.
     * @details VMDString keeps the Shift_JIS byte sequence used by VMD files as its
     *          internal representation. This allows the library to preserve raw bytes
     *          read from VMD string fields as much as possible.
     */
    class VMDString
    {
    public:
        VMDString() = default;

        static VMDString fromUTF8(std::string_view pUTF8String);
        static VMDString fromShiftJIS(std::string_view pShiftJISBytes);
        static VMDString fromShiftJISBytes(const std::byte *pBytes, std::size_t pSize);

        bool empty() const noexcept;
        std::size_t shiftJISByteSize() const noexcept;

        const std::vector<std::byte> &shiftJISBytes() const noexcept;

        std::string toShiftJIS() const;
        std::string toUTF8() const;
        std::string toUTF8ForDisplay(bool pStopAtNul = true) const;

    private:
        std::vector<std::byte> mData;

        explicit VMDString(std::string_view pShiftJISBytes);

        std::string_view shiftJISView() const noexcept;

        static std::size_t shiftJISCharSize(
            std::string_view pBytes,
            std::size_t pPosition) noexcept;

        static std::string decodeShiftJISReplacingInvalidBytes(std::string_view pBytes);
    };
}