#pragma once

#include <string>
#include <string_view>

/**
 * @namespace vmdio::encoding
 * @brief Namespace for string encoding conversion functions.
 */
namespace vmdio::encoding
{
    /**
     * @brief Convert a UTF-8 encoded string to Shift_JIS encoding.
     * @param pString The input string in UTF-8 encoding
     * @return The converted string in Shift_JIS encoding
     * @throws vmdio::exceptions::StringProcessError if Windows API functions show some error,
     *         or if unrepresentable characters are found during conversion to Shift_JIS,
     *         or if the input string is too large to process.
     */
    std::string utf8ToShiftJIS(std::string_view pString);

    /**
     * @brief Convert a Shift_JIS encoded string to UTF-8 encoding.
     * @param pString The input string in Shift_JIS encoding
     * @return The converted string in UTF-8 encoding
     * @throws vmdio::exceptions::StringProcessError if Windows API functions show some error,
     *         or if unrepresentable characters are found during conversion to UTF-8,
     *         or if the input string is too large to process.
     */
    std::string shiftJISToUTF8(std::string_view pString);
}