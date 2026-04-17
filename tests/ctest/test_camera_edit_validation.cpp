#include <vmdio/camera_edit.h>
#include <vmdio/exceptions.h>

#include "test_base.h"

namespace vmd = vmdio::camera_edit;
namespace vmd_except = vmdio::exceptions;

class CameraEditValidationTest : public test_base::TempVmdFileTest
{
};

// TEST: FrameCountOverflow
// Check if FrameOverflowError is properly thrown when the number of frames exceeds
// the maximum limit
TEST_F(CameraEditValidationTest, FrameCountOverflow)
{
    vmd::VMDData lBadData;

    for (size_t i = 0; i < 600001; ++i)
    {
        vmd::CameraFrame lFrame;
        lFrame.frameNumber = static_cast<uint32_t>(i);
        lBadData.cameraFrames.push_back(lFrame);
    }

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameOverflowError);
}

// TEST: CameraFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate camera frames
TEST_F(CameraEditValidationTest, CameraFrameConflict)
{
    vmd::VMDData lBadData;

    vmd::CameraFrame lFrame1;
    lFrame1.frameNumber = 10;
    lBadData.cameraFrames.push_back(lFrame1);

    // Conflict: same frame number as lFrame1
    vmd::CameraFrame lFrame2;
    lFrame2.frameNumber = 10;
    lBadData.cameraFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: InvalidCameraInterpolation
// Check if InvalidFieldValueError is properly thrown when camera interpolation control points have
// out-of-range values
TEST_F(CameraEditValidationTest, InvalidCameraInterpolation)
{
    vmd::VMDData lBadData;

    vmd::CameraFrame lFrame;

    // Invalid interpolation control point value (greater than 127)
    lFrame.interpolation.xPos.x1 = 128;

    lBadData.cameraFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: InvalidProjectionType
// Check if InvalidFieldValueError is properly thrown when a camera frame has an invalid
// projection type value
TEST_F(CameraEditValidationTest, InvalidProjectionType)
{
    vmd::VMDData lBadData;

    vmd::CameraFrame lFrame;

    // Invalid projection type by casting an out-of-range value
    lFrame.projectionType = static_cast<vmd::ProjectionType>(99);

    lBadData.cameraFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: LightFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate light frames
TEST_F(CameraEditValidationTest, LightFrameConflict)
{
    vmd::VMDData lBadData;

    vmd::LightFrame lFrame1;
    lFrame1.frameNumber = 20;
    lBadData.lightFrames.push_back(lFrame1);

    // Conflict: same frame number as lFrame1
    vmd::LightFrame lFrame2;
    lFrame2.frameNumber = 20;
    lBadData.lightFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: SelfShadowFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate self shadow frames
TEST_F(CameraEditValidationTest, SelfShadowFrameConflict)
{
    vmd::VMDData lBadData;

    vmd::SelfShadowFrame lFrame1;
    lFrame1.frameNumber = 30;
    lBadData.selfShadowFrames.push_back(lFrame1);

    // Conflict: same frame number as lFrame1
    vmd::SelfShadowFrame lFrame2;
    lFrame2.frameNumber = 30;
    lBadData.selfShadowFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: InvalidSelfShadowMode
// Check if InvalidFieldValueError is properly thrown when a self shadow frame has an invalid
// self shadow mode
TEST_F(CameraEditValidationTest, InvalidSelfShadowMode)
{
    vmd::VMDData lBadData;

    vmd::SelfShadowFrame lFrame;

    // Invalid mode value by casting an out-of-range value
    lFrame.mode = static_cast<vmd::SelfShadowMode>(99);

    lBadData.selfShadowFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}