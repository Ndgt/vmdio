#pragma once

#include <cmath>
#include <filesystem>
#include <string_view>

#include <gtest/gtest.h>

#include <vmdio/vmd_string.h>

namespace test_base
{
    inline vmdio::VMDString makeVMDString(std::string_view pUTF8String)
    {
        return vmdio::VMDString::fromUTF8(pUTF8String);
    }

    inline bool equalVMDString(
        const vmdio::VMDString &pString1,
        const vmdio::VMDString &pString2)
    {
        return pString1.toShiftJIS() == pString2.toShiftJIS();
    }

    class TempVmdFileTest : public ::testing::Test
    {
    protected:
        std::filesystem::path mTempFilePath = "temp_test_output.vmd";

#ifdef TEST_MOTION_DATA_DIR
        std::filesystem::path mTestMotionDataDir = TEST_MOTION_DATA_DIR;
#else
        std::filesystem::path mTestMotionDataDir = "testdata";
#endif

        void TearDown() override
        {
            if (std::filesystem::exists(mTempFilePath))
                std::filesystem::remove(mTempFilePath);
        }
    };
}