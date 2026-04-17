#include <vmdio/model_edit.h>
#include <vmdio/exceptions.h>

#include "test_base.h"

namespace vmd = vmdio::model_edit;
namespace vmd_except = vmdio::exceptions;

class ModelEditValidationTest : public test_base::TempVmdFileTest
{
};

// TEST: EmptyModelName
// Check if InvalidFieldValueError is properly thrown when the model name in the VMD data is empty
TEST_F(ModelEditValidationTest, EmptyModelName)
{
    vmd::VMDData lBadData;

    // Not setting modelName

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: FrameCountOverflow
// Check if FrameOverflowError is properly thrown when the number of frames exceeds
// the maximum limit
TEST_F(ModelEditValidationTest, FrameCountOverflow)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    // Add more than MAX_FRAME_COUNT(600,000) motion frames to trigger the overflow error
    for (size_t i = 0; i < 600001; ++i)
    {
        vmd::MotionFrame lFrame;
        lFrame.boneName = "ボーン1";
        lFrame.frameNumber = static_cast<uint32_t>(i);
        lBadData.motionFrames.push_back(lFrame);
    }

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameOverflowError);
}

// TEST: MotionFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate motion frames
TEST_F(ModelEditValidationTest, MotionFrameConflict)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MotionFrame lFrame1;
    lFrame1.frameNumber = 10;
    lFrame1.boneName = "ボーン1";
    lBadData.motionFrames.push_back(lFrame1);

    // Conflict: same bone name and frame number as lFrame1
    vmd::MotionFrame lFrame2;
    lFrame2.frameNumber = 10;
    lFrame2.boneName = "ボーン1";
    lBadData.motionFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: EmptyBoneName
// Check if InvalidFieldValueError is properly thrown when a motion frame has an empty bone name
TEST_F(ModelEditValidationTest, EmptyBoneName)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MotionFrame lFrame;

    // Empty bone name
    lFrame.boneName = "";

    lBadData.motionFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: ZeroQuaternion
// Check if InvalidFieldValueError is properly thrown when a motion frame has an quaternion with a
// norm close to zero, which is invalid for rotation data
TEST_F(ModelEditValidationTest, ZeroQuaternion)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MotionFrame lFrame;
    lFrame.boneName = "ボーン1";

    // Set the rotation quaternion to a zero quaternion
    lFrame.rotation = {0.0f, 0.0f, 0.0f, 0.0f};

    lBadData.motionFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: InvalidMotionInterpolation
// Check if InvalidFieldValueError is properly thrown when motion interpolation control points have
// out-of-range values
TEST_F(ModelEditValidationTest, InvalidMotionInterpolation)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MotionFrame lFrame;
    lFrame.boneName = "ボーン1";

    // Invalid interpolation control point value (greater than 127)
    lFrame.interpolation.xPos.x1 = 128;

    lBadData.motionFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: MorphFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate morph frames
TEST_F(ModelEditValidationTest, MorphFrameConflict)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MorphFrame lFrame1;
    lFrame1.frameNumber = 20;
    lFrame1.morphName = "シェイプキー1";
    lBadData.morphFrames.push_back(lFrame1);

    // Conflict: same morph name and frame number as lFrame1
    vmd::MorphFrame lFrame2;
    lFrame2.frameNumber = 20;
    lFrame2.morphName = "シェイプキー1";
    lBadData.morphFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: EmptyMorphName
// Check if InvalidFieldValueError is properly thrown when a morph frame has an empty morph name
TEST_F(ModelEditValidationTest, EmptyMorphName)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::MorphFrame lFrame;

    // Empty morph name
    lFrame.morphName = "";

    lBadData.morphFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: VisibleIKFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate visible IK frames
TEST_F(ModelEditValidationTest, VisibleIKFrameConflict)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::VisibleIKFrame lFrame1;
    lFrame1.frameNumber = 30;
    lBadData.visibleIKFrames.push_back(lFrame1);

    // Conflict: same frame number as lFrame1
    vmd::VisibleIKFrame lFrame2;
    lFrame2.frameNumber = 30;
    lBadData.visibleIKFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: InvalidVisibilityValue
// Check if InvalidFieldValueError is properly thrown when a visible IK frame has an invalid
// visibility value (not Hidden or Visible)
TEST_F(ModelEditValidationTest, InvalidVisibilityValue)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::VisibleIKFrame lFrame;

    // Invalid visibility value by casting an out-of-range value
    lFrame.visibility = static_cast<vmd::Visibility>(99);

    lBadData.visibleIKFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: EmptyIKBoneName
// Check if InvalidFieldValueError is properly thrown when an IK data entry has an empty
// IK bone name
TEST_F(ModelEditValidationTest, EmptyIKBoneName)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::VisibleIKFrame lFrame;

    vmd::IKData lIKData;

    // Empty IK bone name
    lIKData.ikBoneName = "";

    lFrame.ikDataList.push_back(lIKData);
    lBadData.visibleIKFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: InvalidIKState
// Check if InvalidFieldValueError is properly thrown when an IK data entry has an invalid
// IK state value (not OFF or ON)
TEST_F(ModelEditValidationTest, InvalidIKState)
{
    vmd::VMDData lBadData;
    lBadData.modelName = "vmdio-test";

    vmd::VisibleIKFrame lFrame;

    vmd::IKData lIKData;
    lIKData.ikBoneName = "ボーンIK";

    // Invalid IK state by casting an out-of-range value
    lIKData.ikState = static_cast<vmd::IKState>(99);

    lFrame.ikDataList.push_back(lIKData);
    lBadData.visibleIKFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}