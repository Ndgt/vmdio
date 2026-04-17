#include <vmdio/camera_edit.h>
#include <vmdio/exceptions.h>

#include "test_base.h"

namespace vmd = vmdio::camera_edit;

inline bool operator==(
    const vmd::ControlPointSet &pControlPointSet1, const vmd::ControlPointSet &pControlPointSet2)
{
    return pControlPointSet1.x1 == pControlPointSet2.x1 &&
           pControlPointSet1.y1 == pControlPointSet2.y1 &&
           pControlPointSet1.x2 == pControlPointSet2.x2 &&
           pControlPointSet1.y2 == pControlPointSet2.y2;
}

inline bool checkCameraFrame(const vmd::CameraFrame &pFrame1, const vmd::CameraFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check distance
    if (std::abs(pFrame1.distance - pFrame2.distance) > 1e-5)
        return false;

    // Check position
    if (std::abs(pFrame1.position.x - pFrame2.position.x) > 1e-5 ||
        std::abs(pFrame1.position.y - pFrame2.position.y) > 1e-5 ||
        std::abs(pFrame1.position.z - pFrame2.position.z) > 1e-5)
        return false;

    // Check rotation
    if (std::abs(pFrame1.rotation.eulerX - pFrame2.rotation.eulerX) > 1e-5 ||
        std::abs(pFrame1.rotation.eulerY - pFrame2.rotation.eulerY) > 1e-5 ||
        std::abs(pFrame1.rotation.eulerZ - pFrame2.rotation.eulerZ) > 1e-5)
        return false;

    // Check interpolation
    if (!(pFrame1.interpolation.xPos == pFrame2.interpolation.xPos) ||
        !(pFrame1.interpolation.yPos == pFrame2.interpolation.yPos) ||
        !(pFrame1.interpolation.zPos == pFrame2.interpolation.zPos) ||
        !(pFrame1.interpolation.rot == pFrame2.interpolation.rot) ||
        !(pFrame1.interpolation.dist == pFrame2.interpolation.dist) ||
        !(pFrame1.interpolation.view == pFrame2.interpolation.view))
        return false;

    // Check viewing angle
    if (pFrame1.viewingAngle != pFrame2.viewingAngle)
        return false;

    // Check projection type
    if (pFrame1.projectionType != pFrame2.projectionType)
        return false;

    return true;
}

inline bool checkLightFrame(const vmd::LightFrame &pFrame1, const vmd::LightFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check color
    if (pFrame1.color.r != pFrame2.color.r ||
        pFrame1.color.g != pFrame2.color.g ||
        pFrame1.color.b != pFrame2.color.b)
        return false;

    // Check position
    if (std::abs(pFrame1.position.x - pFrame2.position.x) > 1e-5 ||
        std::abs(pFrame1.position.y - pFrame2.position.y) > 1e-5 ||
        std::abs(pFrame1.position.z - pFrame2.position.z) > 1e-5)
        return false;

    return true;
}

inline bool checkSelfShadowFrame(
    const vmd::SelfShadowFrame &pFrame1, const vmd::SelfShadowFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check mode
    if (pFrame1.mode != pFrame2.mode)
        return false;

    // Check shadow range
    if (pFrame1.shadowRange != pFrame2.shadowRange)
        return false;

    return true;
}

class CameraEditRoundtripTest : public test_base::TempVmdFileTest
{
};

// TEST: ReadWriteCameraDataAndConsistency
// Check if readVMD() and writeVMD() do not throw exceptions when handling valid camera data,
// and if the data remains consistent after a read-write-read cycle.
TEST_F(CameraEditRoundtripTest, ReadWriteCameraDataAndConsistency)
{
    std::filesystem::path lTestFilePath = mTestMotionDataDir / "camera.vmd";
    vmd::VMDData lOriginalData;
    vmd::VMDData lReadBackData;

    EXPECT_NO_THROW(lOriginalData = vmd::readVMD(lTestFilePath));
    EXPECT_NO_THROW(vmd::writeVMD(lOriginalData, mTempFilePath));
    EXPECT_NO_THROW(lReadBackData = vmd::readVMD(mTempFilePath));

    EXPECT_EQ(lOriginalData.cameraFrames.size(), lReadBackData.cameraFrames.size());

    for (size_t i = 0; i < lOriginalData.cameraFrames.size(); ++i)
    {
        EXPECT_TRUE(checkCameraFrame(lOriginalData.cameraFrames[i], lReadBackData.cameraFrames[i]));
    }
}

// TEST: ReadWriteLightDataAndConsistency
// Check if readVMD() and writeVMD() do not throw exceptions when handling valid light data,
// and if the data remains consistent after a read-write-read cycle.
TEST_F(CameraEditRoundtripTest, ReadWriteLightDataAndConsistency)
{
    std::filesystem::path lTestFilePath = mTestMotionDataDir / "light.vmd";
    vmd::VMDData lOriginalData;
    vmd::VMDData lReadBackData;

    EXPECT_NO_THROW(lOriginalData = vmd::readVMD(lTestFilePath));
    EXPECT_NO_THROW(vmd::writeVMD(lOriginalData, mTempFilePath));
    EXPECT_NO_THROW(lReadBackData = vmd::readVMD(mTempFilePath));

    EXPECT_EQ(lOriginalData.lightFrames.size(), lReadBackData.lightFrames.size());

    for (size_t i = 0; i < lOriginalData.lightFrames.size(); ++i)
    {
        EXPECT_TRUE(checkLightFrame(lOriginalData.lightFrames[i], lReadBackData.lightFrames[i]));
    }
}

// TEST: ReadWriteSelfShadowDataAndConsistency
// Check if readVMD() and writeVMD() do not throw exceptions when handling valid self shadow data,
// and if the data remains consistent after a read-write-read cycle.
TEST_F(CameraEditRoundtripTest, ReadWriteSelfShadowDataAndConsistency)
{
    std::filesystem::path lTestFilePath = mTestMotionDataDir / "selfshadow.vmd";
    vmd::VMDData lOriginalData;
    vmd::VMDData lReadBackData;

    EXPECT_NO_THROW(lOriginalData = vmd::readVMD(lTestFilePath));
    EXPECT_NO_THROW(vmd::writeVMD(lOriginalData, mTempFilePath));
    EXPECT_NO_THROW(lReadBackData = vmd::readVMD(mTempFilePath));

    EXPECT_EQ(lOriginalData.selfShadowFrames.size(), lReadBackData.selfShadowFrames.size());

    for (size_t i = 0; i < lOriginalData.selfShadowFrames.size(); ++i)
    {
        EXPECT_TRUE(checkSelfShadowFrame(lOriginalData.selfShadowFrames[i], lReadBackData.selfShadowFrames[i]));
    }
}