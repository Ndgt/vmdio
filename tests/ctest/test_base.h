#pragma once

#include <cmath>
#include <filesystem>

#include <gtest/gtest.h>

namespace test_base
{
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