#include <vmdio/model_edit.h>
#include <vmdio/camera_edit.h>
#include <vmdio/exceptions.h>

#include "test_base.h"

namespace vmd_model = vmdio::model_edit;
namespace vmd_camera = vmdio::camera_edit;
namespace vmd_except = vmdio::exceptions;

class FormatMismatchTest : public test_base::TempVmdFileTest
{
};

TEST_F(FormatMismatchTest, ReadCameraEditVmdWithModelEditReader)
{
    std::filesystem::path lCameraVmdPath = mTestMotionDataDir / "camera.vmd";

    EXPECT_THROW(vmd_model::readVMD(lCameraVmdPath), vmd_except::IncompatibleFormatError);
}

TEST_F(FormatMismatchTest, WriteCameraEditModelNameWithModelEditWriter)
{
    vmd_model::VMDData lBadData;
    lBadData.modelName = "カメラ・照明";

    EXPECT_THROW(
        vmd_model::writeVMD(lBadData, mTempFilePath), vmd_except::IncompatibleFormatError);
}

TEST_F(FormatMismatchTest, ReadModelEditVmdWithCameraEditReader)
{
    std::filesystem::path lModelEditVmdPath = mTestMotionDataDir / "motion_morph_visibleik.vmd";

    EXPECT_THROW(vmd_camera::readVMD(lModelEditVmdPath), vmd_except::IncompatibleFormatError);
}