#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "vmd_exceptions.h"
#include "vmd_string.h"
#include "private/vmd_constants.h"

namespace vmdio::internal
{
    inline void addAndValidateFrameCount(uint64_t &pTotalFrameCount, uint32_t pFrameCountToAdd)
    {
        pTotalFrameCount += pFrameCountToAdd;

        if (pTotalFrameCount > static_cast<uint64_t>(MAX_FRAME_COUNT))
        {
            throw exceptions::FrameOverflowError(
                "Total frame count that library will read from the VMD file exceeds the supported maximum limit of " + std::to_string(MAX_FRAME_COUNT) +
                ". Total frames: " + std::to_string(pTotalFrameCount));
        }
    }

    inline std::string readHeader(std::ifstream &pStream)
    {
        char lHeader[internal::VMD_HEADER_SIZE];
        pStream.read(lHeader, sizeof(lHeader));

        return std::string(lHeader, strnlen(lHeader, sizeof(lHeader)));
    }

    inline void writeHeader(std::ofstream &pStream)
    {
        char lHeaderStr[internal::VMD_HEADER_SIZE] = {};
        std::memcpy(lHeaderStr, internal::VMD_HEADER.data(), internal::VMD_HEADER.size());
        pStream.write(lHeaderStr, sizeof(lHeaderStr));
    }

    inline VMDString readStringField(std::ifstream &pStream, std::size_t pFieldSize)
    {
        std::string lBuffer(pFieldSize, '\0');

        if (pFieldSize != 0)
            pStream.read(lBuffer.data(), static_cast<std::streamsize>(lBuffer.size()));

        const std::size_t lNulPosition = lBuffer.find('\0');

        if (lNulPosition != std::string::npos)
            lBuffer.resize(lNulPosition);

        return VMDString::fromShiftJIS(lBuffer);
    }

    inline void writeStringField(
        std::ofstream &pStream, const VMDString &pString, std::size_t pFieldSize,
        std::string_view pLabel)
    {
        const std::vector<std::byte> &lBytes = pString.shiftJISBytes();

        if (lBytes.size() > pFieldSize)
            throw exceptions::InvalidFieldValueError(
                std::string(pLabel) +
                " exceeds the field size limit of " + std::to_string(pFieldSize) +
                " bytes. Actual byte size: " + std::to_string(lBytes.size()));

        std::vector<std::byte> lBuffer(pFieldSize, std::byte{0});

        if (!lBytes.empty())
            std::memcpy(lBuffer.data(), lBytes.data(), lBytes.size());

        pStream.write(
            reinterpret_cast<const char *>(lBuffer.data()),
            static_cast<std::streamsize>(lBuffer.size()));
    }

    inline void readFlagValue(std::ifstream &pStream, uint8_t &pFlag)
    {
        pStream.read(reinterpret_cast<char *>(&pFlag), sizeof(pFlag));
    }

    inline void writeFlagValue(std::ofstream &pStream, uint8_t &pFlag)
    {
        pStream.write(reinterpret_cast<const char *>(&pFlag), sizeof(pFlag));
    }

    inline uint32_t readRaw32bits(std::ifstream &pStream)
    {
        uint8_t lBuffer[4];
        pStream.read(reinterpret_cast<char *>(lBuffer), sizeof(lBuffer));

        // Combine bytes into a 32-bit unsigned integer (little-endian)
        return static_cast<uint32_t>(lBuffer[0]) |
               (static_cast<uint32_t>(lBuffer[1]) << 8) |
               (static_cast<uint32_t>(lBuffer[2]) << 16) |
               (static_cast<uint32_t>(lBuffer[3]) << 24);
    }

    inline void writeRaw32bits(std::ofstream &pStream, uint32_t pRaw)
    {
        // Split the 32-bit unsigned integer (little-endian) into bytes
        uint8_t lBuffer[4] = {
            static_cast<uint8_t>(pRaw & 0xFFu),
            static_cast<uint8_t>((pRaw >> 8) & 0xFFu),
            static_cast<uint8_t>((pRaw >> 16) & 0xFFu),
            static_cast<uint8_t>((pRaw >> 24) & 0xFFu),
        };

        pStream.write(reinterpret_cast<const char *>(lBuffer), sizeof(lBuffer));
    }

    inline void readUintValue(std::ifstream &pStream, uint32_t &pValue)
    {
        pValue = readRaw32bits(pStream);
    }

    inline void writeUintValue(std::ofstream &pStream, uint32_t pValue)
    {
        writeRaw32bits(pStream, pValue);
    }

    inline void readIntValue(std::ifstream &pStream, int32_t &pValue)
    {
        const uint32_t lRawValue = readRaw32bits(pStream);
        std::memcpy(&pValue, &lRawValue, sizeof(pValue));
    }

    inline void writeIntValue(std::ofstream &pStream, int32_t pValue)
    {
        uint32_t lRawValue;
        std::memcpy(&lRawValue, &pValue, sizeof(lRawValue));
        writeRaw32bits(pStream, lRawValue);
    }

    inline void readFloatValue(std::ifstream &pStream, float &pValue)
    {
        const uint32_t lRawValue = readRaw32bits(pStream);
        std::memcpy(&pValue, &lRawValue, sizeof(pValue));
    }

    inline void writeFloatValue(std::ofstream &pStream, float pValue)
    {
        uint32_t lRawValue;
        std::memcpy(&lRawValue, &pValue, sizeof(lRawValue));
        writeRaw32bits(pStream, lRawValue);
    }

    inline void readInterpolationBuffer(std::ifstream &pStream, uint8_t *pBuffer, std::size_t pSize)
    {
        pStream.read(reinterpret_cast<char *>(pBuffer), pSize);
    }

    inline void writeInterpolationBuffer(
        std::ofstream &pStream, const uint8_t *pBuffer, std::size_t pSize)
    {
        pStream.write(reinterpret_cast<const char *>(pBuffer), pSize);
    }

    template <typename ControlPointSetT>
    inline void validateControlPointSet(
        const ControlPointSetT &pControlPointSet, std::string_view pControlPointSetName,
        uint32_t pFrameNumber)
    {
        const auto checkOne = [&](uint32_t pValue, const char *pFieldName)
        {
            if (pValue > 127)
            {
                throw exceptions::InvalidFieldValueError(
                    std::string(pControlPointSetName) + "." + pFieldName +
                    " must be in the range of 0 to 127. Invalid value: " +
                    std::to_string(pValue) +
                    " Frame number: " + std::to_string(pFrameNumber));
            }
        };

        checkOne(pControlPointSet.x1, "x1");
        checkOne(pControlPointSet.y1, "y1");
        checkOne(pControlPointSet.x2, "x2");
        checkOne(pControlPointSet.y2, "y2");
    }
}