#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

#include "test_base.h"

namespace vmd = vmdio::model_edit;

inline bool operator==(
    const vmd::ControlPointSet &pControlPointSet1, const vmd::ControlPointSet &pControlPointSet2)
{
    return pControlPointSet1.x1 == pControlPointSet2.x1 &&
           pControlPointSet1.y1 == pControlPointSet2.y1 &&
           pControlPointSet1.x2 == pControlPointSet2.x2 &&
           pControlPointSet1.y2 == pControlPointSet2.y2;
}

inline bool checkMotionFrame(const vmd::MotionFrame &pFrame1, const vmd::MotionFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check bone name
    if (!test_base::equalVMDString(pFrame1.boneName, pFrame2.boneName))
        return false;

    // Check position
    if (std::abs(pFrame1.position.x - pFrame2.position.x) > 1e-5 ||
        std::abs(pFrame1.position.y - pFrame2.position.y) > 1e-5 ||
        std::abs(pFrame1.position.z - pFrame2.position.z) > 1e-5)
        return false;

    // Check rotation
    if (std::abs(pFrame1.rotation.qx - pFrame2.rotation.qx) > 1e-5 ||
        std::abs(pFrame1.rotation.qy - pFrame2.rotation.qy) > 1e-5 ||
        std::abs(pFrame1.rotation.qz - pFrame2.rotation.qz) > 1e-5 ||
        std::abs(pFrame1.rotation.qw - pFrame2.rotation.qw) > 1e-5)
        return false;

    // Check interpolation
    if (!(pFrame1.interpolation.xPos == pFrame2.interpolation.xPos) ||
        !(pFrame1.interpolation.yPos == pFrame2.interpolation.yPos) ||
        !(pFrame1.interpolation.zPos == pFrame2.interpolation.zPos) ||
        !(pFrame1.interpolation.rot == pFrame2.interpolation.rot))
        return false;

    return true;
}

inline bool checkMorphFrame(const vmd::MorphFrame &pFrame1, const vmd::MorphFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check morph name
    if (!test_base::equalVMDString(pFrame1.morphName, pFrame2.morphName))
        return false;

    // Check value
    if (std::abs(pFrame1.value - pFrame2.value) > 1e-5)
        return false;

    return true;
}

inline bool checkVisibleIKFrame(
    const vmd::VisibleIKFrame &pFrame1, const vmd::VisibleIKFrame &pFrame2)
{
    // Check frame number
    if (pFrame1.frameNumber != pFrame2.frameNumber)
        return false;

    // Check visible flag
    if (pFrame1.visibility != pFrame2.visibility)
        return false;

    // Check IK data list size
    if (pFrame1.ikDataList.size() != pFrame2.ikDataList.size())
        return false;

    // Check IK data list content
    for (size_t i = 0; i < pFrame1.ikDataList.size(); ++i)
    {
        const vmd::IKData &lIKData1 = pFrame1.ikDataList[i];
        const vmd::IKData &lIKData2 = pFrame2.ikDataList[i];

        if (!test_base::equalVMDString(lIKData1.ikBoneName, lIKData2.ikBoneName) ||
            lIKData1.ikState != lIKData2.ikState)
            return false;
    }

    return true;
}

class ModelEditRoundtripTest : public test_base::TempVmdFileTest
{
};

// TEST: ReadWriteDataAndConsistency
// Check if readVMD() and writeVMD() do not throw exceptions when handling valid data,
// and if the data remains consistent after a read-write-read cycle.
TEST_F(ModelEditRoundtripTest, ReadWriteDataAndConsistency)
{
    std::filesystem::path lTestFilePath = mTestMotionDataDir / "motion_morph_visibleik.vmd";
    vmd::VMDData lOriginalData;
    vmd::VMDData lReadBackData;

    EXPECT_NO_THROW(lOriginalData = vmd::readVMD(lTestFilePath));
    EXPECT_NO_THROW(vmd::writeVMD(lOriginalData, mTempFilePath));
    EXPECT_NO_THROW(lReadBackData = vmd::readVMD(mTempFilePath));

    // Check if the model name is the same
    EXPECT_TRUE(test_base::equalVMDString(lOriginalData.modelName, lReadBackData.modelName));

    // Check if the frame counts are the same
    EXPECT_EQ(lOriginalData.motionFrames.size(), lReadBackData.motionFrames.size());
    EXPECT_EQ(lOriginalData.morphFrames.size(), lReadBackData.morphFrames.size());
    EXPECT_EQ(lOriginalData.visibleIKFrames.size(), lReadBackData.visibleIKFrames.size());

    // Check if the content of each motion, morph, and visible IK frame is the same
    for (size_t i = 0; i < lOriginalData.motionFrames.size(); ++i)
    {
        EXPECT_TRUE(checkMotionFrame(lOriginalData.motionFrames[i], lReadBackData.motionFrames[i]));
    }

    for (size_t i = 0; i < lOriginalData.morphFrames.size(); ++i)
    {
        EXPECT_TRUE(checkMorphFrame(lOriginalData.morphFrames[i], lReadBackData.morphFrames[i]));
    }

    for (size_t i = 0; i < lOriginalData.visibleIKFrames.size(); ++i)
    {
        EXPECT_TRUE(checkVisibleIKFrame(
            lOriginalData.visibleIKFrames[i], lReadBackData.visibleIKFrames[i]));
    }
}