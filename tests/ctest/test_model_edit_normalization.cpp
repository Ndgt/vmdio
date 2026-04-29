#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

#include "test_base.h"

namespace vmd = vmdio::model_edit;
namespace vmd_except = vmdio::exceptions;

class ModelEditNormalizationTest : public test_base::TempVmdFileTest
{
};

// TEST: NonUnitQuaternionNormalization
// Check if non-unit quaternions in motion frames are properly normalized when writing VMD data
TEST_F(ModelEditNormalizationTest, NonUnitQuaternionNormalization)
{
    vmd::VMDData lOriginalData;
    vmd::VMDData lReadBackData;

    lOriginalData.modelName = test_base::makeVMDString("vmdio-test");

    vmd::MotionFrame lFrame;
    lFrame.boneName = test_base::makeVMDString(u8"ボーン1");
    lFrame.rotation.qx = 0.0f;
    lFrame.rotation.qy = 0.0f;
    lFrame.rotation.qz = 0.0f;
    lFrame.rotation.qw = 2.0f; // Non-unit quaternion with norm of 2.0

    lOriginalData.motionFrames.push_back(lFrame);

    vmd::writeVMD(lOriginalData, mTempFilePath);
    lReadBackData = vmd::readVMD(mTempFilePath);

    // Check if the original quaternion is not changed
    EXPECT_FLOAT_EQ(lOriginalData.motionFrames[0].rotation.qx, 0.0f);
    EXPECT_FLOAT_EQ(lOriginalData.motionFrames[0].rotation.qy, 0.0f);
    EXPECT_FLOAT_EQ(lOriginalData.motionFrames[0].rotation.qz, 0.0f);
    EXPECT_FLOAT_EQ(lOriginalData.motionFrames[0].rotation.qw, 2.0f);

    // Check if the read-back quaternion is normalized to a unit quaternion
    EXPECT_FLOAT_EQ(lReadBackData.motionFrames[0].rotation.qx, 0.0f);
    EXPECT_FLOAT_EQ(lReadBackData.motionFrames[0].rotation.qy, 0.0f);
    EXPECT_FLOAT_EQ(lReadBackData.motionFrames[0].rotation.qz, 0.0f);
    EXPECT_FLOAT_EQ(lReadBackData.motionFrames[0].rotation.qw, 1.0f);
}