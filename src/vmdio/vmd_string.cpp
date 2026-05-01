#include "vmd_string.h"

#include <cstring>

#include "vmd_encoding.h"
#include "vmd_exceptions.h"

namespace vmdio
{
    VMDString::VMDString(std::string_view pShiftJISsv) : mData(pShiftJISsv.size())
    {
        if (!pShiftJISsv.empty())
            std::memcpy(mData.data(), pShiftJISsv.data(), pShiftJISsv.size());
    }

    const std::vector<std::byte> &VMDString::shiftJISBytes() const noexcept
    {
        return mData;
    }

    bool VMDString::empty() const noexcept
    {
        return mData.empty();
    }

    std::size_t VMDString::sizeofShiftJISBytes() const noexcept
    {
        return mData.size();
    }

    VMDString VMDString::fromShiftJIS(std::string_view pShiftJISsv)
    {
        return VMDString{pShiftJISsv};
    }

    VMDString VMDString::fromUTF8(std::string_view pUTF8Encodedsv)
    {
        const std::string lShiftJISString = encoding::utf8ToShiftJIS(pUTF8Encodedsv);

        return VMDString{lShiftJISString};
    }

    std::string VMDString::toShiftJIS() const
    {
        std::string lResult(mData.size(), '\0');

        if (!mData.empty())
            std::memcpy(lResult.data(), mData.data(), mData.size());

        return lResult;
    }

    std::string VMDString::toUTF8() const
    {
        return encoding::shiftJISToUTF8(shiftJISView());
    }

    std::string VMDString::toUTF8ForDisplay(bool pStopAtNul) const
    {
        std::string_view lView = shiftJISView();

        if (pStopAtNul)
        {
            const std::size_t lNulPosition = lView.find('\0');

            if (lNulPosition != std::string_view::npos)
                lView = lView.substr(0, lNulPosition);
        }

        return decodeShiftJISReplacingInvalidBytes(lView);
    }

    std::string_view VMDString::shiftJISView() const noexcept
    {
        if (mData.empty())
            return {};

        return std::string_view{reinterpret_cast<const char *>(mData.data()), mData.size()};
    }

    std::size_t VMDString::shiftJISCharSize(
        std::string_view pBytes, std::size_t pPosition) noexcept
    {
        if (pPosition >= pBytes.size())
            return 0;

        const unsigned char lFirstByte = static_cast<unsigned char>(pBytes[pPosition]);

        if ((lFirstByte <= 0x7F) || (0xA1 <= lFirstByte && lFirstByte <= 0xDF))
            return 1;

        if ((0x81 <= lFirstByte && lFirstByte <= 0x9F) ||
            (0xE0 <= lFirstByte && lFirstByte <= 0xFC))
        {
            if (pPosition + 1 < pBytes.size())
            {
                const unsigned char lSecondByte =
                    static_cast<unsigned char>(pBytes[pPosition + 1]);

                if ((0x40 <= lSecondByte && lSecondByte <= 0x7E) ||
                    (0x80 <= lSecondByte && lSecondByte <= 0xFC))
                    return 2;
            }

            return 0;
        }

        return 0;
    }

    std::string VMDString::decodeShiftJISReplacingInvalidBytes(std::string_view pBytes)
    {
        std::string lResult;
        std::size_t lPosition = 0;

        while (lPosition < pBytes.size())
        {
            const std::size_t lCharSize = shiftJISCharSize(pBytes, lPosition);

            if (lCharSize == 0)
            {
                lResult.push_back('?');
                ++lPosition;
                continue;
            }

            const std::string_view lOneCharacter{pBytes.data() + lPosition, lCharSize};

            try
            {
                lResult += encoding::shiftJISToUTF8(lOneCharacter);
            }
            catch (const exceptions::StringProcessError &)
            {
                lResult.push_back('?');
            }

            lPosition += lCharSize;
        }

        return lResult;
    }
}