#include "encoding.h"

#include <windows.h>

#include "exceptions.h"

namespace
{
    inline std::string fromUtf16(std::wstring_view pUtf16String, UINT pCodePage)
    {
        if (pUtf16String.empty())
            return {};

        if (pUtf16String.size() > static_cast<size_t>(INT_MAX))
            throw vmdio::exceptions::StringProcessError("Input string is too large to process.");

        // Use WC_ERR_INVALID_CHARS flag for UTF-8 encoding to ensure that the function fails
        // if unrepresentable characters are found
        DWORD lDwFlags = (pCodePage == CP_UTF8) ? WC_ERR_INVALID_CHARS : 0;

        // Calculate the required buffer size for the encoded string
        int lRequiredSizeInBytes = WideCharToMultiByte(
            pCodePage, lDwFlags,
            pUtf16String.data(), static_cast<int>(pUtf16String.size()),
            nullptr, 0,
            nullptr, nullptr);

        if (lRequiredSizeInBytes == 0)
        {
            DWORD lErrorCode = GetLastError();
            throw vmdio::exceptions::StringProcessError(
                "Internal string process error: WideCharToMultiByte returned 0 when calculating required buffer size. Error code: " + std::to_string(lErrorCode));
        }

        std::string lEncodedString(lRequiredSizeInBytes, '\0');

        // Set up the default character used flag for Shift_JIS encoding
        BOOL lDefaultCharUsed = FALSE;
        BOOL *lLpUsedDefaultChar = (pCodePage == CP_UTF8) ? nullptr : &lDefaultCharUsed;

        // Convert the UTF-16 string to the specified encoding
        int lWrittenLength = WideCharToMultiByte(
            pCodePage, lDwFlags,
            pUtf16String.data(), static_cast<int>(pUtf16String.size()),
            lEncodedString.data(), lRequiredSizeInBytes,
            nullptr, lLpUsedDefaultChar);

        if (lWrittenLength == 0)
        {
            DWORD lErrorCode = GetLastError();
            throw vmdio::exceptions::StringProcessError(
                "Internal string process error: WideCharToMultiByte returned 0 during conversion. Error code: " + std::to_string(lErrorCode));
        }

        if (lLpUsedDefaultChar && *lLpUsedDefaultChar)
        {
            throw vmdio::exceptions::StringProcessError(
                "Internal string process error: WideCharToMultiByte detected unrepresentable characters during conversion to Shift_JIS.");
        }

        return lEncodedString;
    }

    inline std::wstring toUtf16(std::string_view pEncodedString, UINT pCodePage)
    {
        if (pEncodedString.empty())
            return {};

        if (pEncodedString.size() > static_cast<size_t>(INT_MAX))
            throw vmdio::exceptions::StringProcessError("Input string is too large to process.");

        // Calculate the required buffer size for the UTF-16 string
        int lRequiredLength = MultiByteToWideChar(
            pCodePage,
            MB_ERR_INVALID_CHARS, // Always fail if invalid characters are found
            pEncodedString.data(), static_cast<int>(pEncodedString.size()),
            nullptr, 0);

        if (lRequiredLength == 0)
        {
            DWORD lErrorCode = GetLastError();
            throw vmdio::exceptions::StringProcessError(
                "Internal string process error: MultiByteToWideChar returned 0 when calculating required buffer size. Error code: " + std::to_string(lErrorCode));
        }

        std::wstring lUTF16String(lRequiredLength, L'\0');

        // Convert the encoded string to UTF-16
        int lWrittenLength = MultiByteToWideChar(
            pCodePage,
            MB_ERR_INVALID_CHARS,
            pEncodedString.data(), static_cast<int>(pEncodedString.size()),
            lUTF16String.data(), lRequiredLength);

        if (lWrittenLength == 0)
        {
            DWORD lErrorCode = GetLastError();
            throw vmdio::exceptions::StringProcessError(
                "Internal string process error: MultiByteToWideChar returned 0 during conversion. Error code: " + std::to_string(lErrorCode));
        }

        return lUTF16String;
    }
}

namespace vmdio::encoding
{
    std::string utf8ToShiftJIS(std::string_view pString)
    {
        return fromUtf16(toUtf16(pString, CP_UTF8), 932);
    }

    std::string shiftJISToUTF8(std::string_view pString)
    {
        return fromUtf16(toUtf16(pString, 932), CP_UTF8);
    }
}