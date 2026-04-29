import pytest

import pyvmdio.model_edit as vmdio_model
import pyvmdio.camera_edit as vmdio_camera
import pyvmdio.exceptions as vmdio_exceptions
from pyvmdio.vmd_string import VMDString


def test_read_camera_edit_vmd_as_model_edit(test_motion_data_dir):
    src = test_motion_data_dir / "camera.vmd"
    with pytest.raises(vmdio_exceptions.IncompatibleFormatError):
        vmdio_model.readVMD(src)


def test_write_camera_edit_vmd_from_model_edit(temp_vmd_path):
    dummy = vmdio_model.VMDData()
    dummy.modelName = VMDString.fromUTF8("カメラ・照明")

    with pytest.raises(vmdio_exceptions.IncompatibleFormatError):
        vmdio_model.writeVMD(dummy, temp_vmd_path)


def test_read_model_edit_vmd_as_camera_edit(test_motion_data_dir):
    src = test_motion_data_dir / "motion_morph_visibleik.vmd"
    with pytest.raises(vmdio_exceptions.IncompatibleFormatError):
        vmdio_camera.readVMD(src)