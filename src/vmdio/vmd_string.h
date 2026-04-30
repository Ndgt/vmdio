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
        /**
         * @brief Default constructor. Creates an empty VMDString.
         */
        VMDString() = default;

        /**
         * @brief Returns a reference to the internal Shift_JIS byte sequence.
         * @return A const reference to the vector of bytes representing the Shift_JIS string
         */
        const std::vector<std::byte> &shiftJISBytes() const noexcept;

        /**
         * @brief Checks if the VMDString is empty.
         * @return true if the VMDString is empty, false otherwise
         */
        bool empty() const noexcept;

        /**
         * @brief Returns the size of the internal Shift_JIS byte sequence.
         * @return The size of the Shift_JIS byte sequence
         */
        std::size_t sizeofShiftJISBytes() const noexcept;

        /**
         * @brief Creates a VMDString from a Shift_JIS encoded string.
         * @param pShiftJISsv The Shift_JIS encoded string view
         * @return A VMDString containing the Shift_JIS byte sequence
         */
        static VMDString fromShiftJIS(std::string_view pShiftJISsv);

        /**
         * @brief Creates a VMDString from a UTF-8 encoded string.
         * @param pUTF8Encodedsv The UTF-8 encoded string view
         * @return A VMDString containing the Shift_JIS byte sequence
         */
        static VMDString fromUTF8(std::string_view pUTF8Encodedsv);

        /**
         * @brief Converts the VMDString to a Shift_JIS encoded string.
         * @return A std::string containing the Shift_JIS encoded string
         */
        std::string toShiftJIS() const;

        /**
         * @brief Converts the VMDString to a UTF-8 encoded string.
         * @return A std::string containing the UTF-8 encoded string
         * @throws vmdio::exceptions::StringProcessError If UTF-8 decoding or conversion
         *         to Shift_JIS fails.
         */
        std::string toUTF8() const;

        /**
         * @brief Converts the VMDString to a UTF-8 encoded string for display purposes.
         * @param pStopAtNul Whether to stop at the first null byte
         * @return A std::string containing the UTF-8 encoded string
         */
        std::string toUTF8ForDisplay(bool pStopAtNul = true) const;

    private:
        /**
         * @brief Internal constructor that initializes the VMDString with a Shift_JIS encoded
         *        string view.
         * @param pShiftJISsv The Shift_JIS encoded string view
         */
        explicit VMDString(std::string_view pShiftJISsv);

        /**
         * @brief Returns a string view of the internal Shift_JIS byte sequence.
         * @return A string view representing the Shift_JIS byte sequence
         */
        std::string_view shiftJISView() const noexcept;

        /**
         * @brief Determines the size of a Shift_JIS character starting at a given position
         *        in a byte sequence.
         * @param pBytes The byte sequence to analyze
         * @param pPosition The position in the byte sequence to check
         * @return The size of the Shift_JIS character in bytes, or 0 if the bytes at the position
         *         do not form a valid Shift_JIS character
         */
        static std::size_t shiftJISCharSize(
            std::string_view pBytes, std::size_t pPosition) noexcept;

        /**
         * @brief Decodes a Shift_JIS encoded byte sequence to a UTF-8 string, replacing any invalid
         *        byte sequences with '?' characters.
         * @param pBytes The Shift_JIS encoded byte sequence to decode
         * @return A UTF-8 encoded string with invalid byte sequences replaced
         * @note Use this function for display purposes.
         */
        static std::string decodeShiftJISReplacingInvalidBytes(std::string_view pBytes);

    private:
        std::vector<std::byte> mData; ///< Internal storage for the Shift_JIS byte sequence
    };
}