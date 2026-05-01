#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

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

// TEST: ModelNameTooLong
// Check if InvalidFieldValueError is properly thrown when modelName exceeds the VMD field limit
TEST_F(ModelEditValidationTest, ModelNameTooLong)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("123456789012345678901");

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: FrameCountOverflow
// Check if FrameOverflowError is properly thrown when the number of frames exceeds
// the maximum limit
TEST_F(ModelEditValidationTest, FrameCountOverflow)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    // Add more than MAX_FRAME_COUNT(600,000) motion frames to trigger the overflow error
    for (size_t i = 0; i < 600001; ++i)
    {
        vmd::MotionFrame lFrame;
        lFrame.boneName = test_base::makeVMDString(u8"ボーン1");
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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame1;
    lFrame1.frameNumber = 10;
    lFrame1.boneName = test_base::makeVMDString(u8"ボーン1");
    lBadData.motionFrames.push_back(lFrame1);

    // Conflict: same bone name and frame number as lFrame1
    vmd::MotionFrame lFrame2;
    lFrame2.frameNumber = 10;
    lFrame2.boneName = test_base::makeVMDString(u8"ボーン1");
    lBadData.motionFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: EmptyBoneName
// Check if InvalidFieldValueError is properly thrown when a motion frame has an empty bone name
TEST_F(ModelEditValidationTest, EmptyBoneName)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame;

    // Empty bone name
    lFrame.boneName = test_base::makeVMDString("");

    lBadData.motionFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: BoneNameTooLong
// Check if InvalidFieldValueError is properly thrown when boneName exceeds the VMD field limit
TEST_F(ModelEditValidationTest, BoneNameTooLong)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame;
    lFrame.boneName = test_base::makeVMDString("1234567890123456");

    lBadData.motionFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: ZeroQuaternion
// Check if InvalidFieldValueError is properly thrown when a motion frame has an quaternion with a
// norm close to zero, which is invalid for rotation data
TEST_F(ModelEditValidationTest, ZeroQuaternion)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame;
    lFrame.boneName = test_base::makeVMDString(u8"ボーン1");

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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame;
    lFrame.boneName = test_base::makeVMDString(u8"ボーン1");

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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MorphFrame lFrame1;
    lFrame1.frameNumber = 20;
    lFrame1.morphName = test_base::makeVMDString(u8"シェイプキー1");
    lBadData.morphFrames.push_back(lFrame1);

    // Conflict: same morph name and frame number as lFrame1
    vmd::MorphFrame lFrame2;
    lFrame2.frameNumber = 20;
    lFrame2.morphName = test_base::makeVMDString(u8"シェイプキー1");
    lBadData.morphFrames.push_back(lFrame2);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::FrameConflictError);
}

// TEST: EmptyMorphName
// Check if InvalidFieldValueError is properly thrown when a morph frame has an empty morph name
TEST_F(ModelEditValidationTest, EmptyMorphName)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MorphFrame lFrame;

    // Empty morph name
    lFrame.morphName = test_base::makeVMDString("");

    lBadData.morphFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: MorphNameTooLong
// Check if InvalidFieldValueError is properly thrown when morphName exceeds the VMD field limit
TEST_F(ModelEditValidationTest, MorphNameTooLong)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MorphFrame lFrame;
    lFrame.morphName = test_base::makeVMDString("123456789012345678901");

    lBadData.morphFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: VisibleIKFrameConflict
// Check if FrameConflictError is properly thrown when there are duplicate visible IK frames
TEST_F(ModelEditValidationTest, VisibleIKFrameConflict)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::VisibleIKFrame lFrame;

    vmd::IKData lIKData;

    // Empty IK bone name
    lIKData.ikBoneName = test_base::makeVMDString("");

    lFrame.ikDataList.push_back(lIKData);
    lBadData.visibleIKFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}

// TEST: IKBoneNameTooLong
// Check if InvalidFieldValueError is properly thrown when an IK data entry has an IK bone name
// that exceeds the VMD field limit
TEST_F(ModelEditValidationTest, IKBoneNameTooLong)
{
    vmd::VMDData lBadData;
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::VisibleIKFrame lFrame;

    vmd::IKData lIKData;
    lIKData.ikBoneName = test_base::makeVMDString("123456789012345678901");

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
    lBadData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::VisibleIKFrame lFrame;

    vmd::IKData lIKData;
    lIKData.ikBoneName = test_base::makeVMDString(u8"ボーンIK");

    // Invalid IK state by casting an out-of-range value
    lIKData.ikState = static_cast<vmd::IKState>(99);

    lFrame.ikDataList.push_back(lIKData);
    lBadData.visibleIKFrames.push_back(lFrame);

    EXPECT_THROW(vmd::writeVMD(lBadData, mTempFilePath), vmd_except::InvalidFieldValueError);
}