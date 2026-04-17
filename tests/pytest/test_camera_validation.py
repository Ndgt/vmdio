import pytest

import pyvmdio.camera_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions


def test_frame_count_overflow(temp_vmd_path):
    bad_data = vmdio.VMDData()
    for i in range(600001):
        frame = vmdio.CameraFrame()
        frame.frameNumber = i
        bad_data.cameraFrames.append(frame)

    with pytest.raises(vmdio_exceptions.FrameOverflowError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_camera_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()

    f1 = vmdio.CameraFrame()
    f1.frameNumber = 10
    bad_data.cameraFrames.append(f1)

    f2 = vmdio.CameraFrame()
    f2.frameNumber = 10
    bad_data.cameraFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_invalid_camera_interpolation(temp_vmd_path):
    bad_data = vmdio.VMDData()
    frame = vmdio.CameraFrame()

    frame.interpolation.xPos.x1 = 128
    bad_data.cameraFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_invalid_projection_type(temp_vmd_path):
    bad_data = vmdio.VMDData()
    frame = vmdio.CameraFrame()

    try:
        frame.projectionType = 99  # Pybind11 might raise TypeError here
    except TypeError:
        return

    bad_data.cameraFrames.append(frame)
    with pytest.raises((vmdio_exceptions.InvalidFieldValueError, TypeError)):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_light_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()

    f1 = vmdio.LightFrame()
    f1.frameNumber = 20
    bad_data.lightFrames.append(f1)

    f2 = vmdio.LightFrame()
    f2.frameNumber = 20
    bad_data.lightFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_self_shadow_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()

    f1 = vmdio.SelfShadowFrame()
    f1.frameNumber = 30
    bad_data.selfShadowFrames.append(f1)

    f2 = vmdio.SelfShadowFrame()
    f2.frameNumber = 30
    bad_data.selfShadowFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_invalid_self_shadow_mode(temp_vmd_path):
    bad_data = vmdio.VMDData()
    frame = vmdio.SelfShadowFrame()

    try:
        frame.mode = 99
    except TypeError:
        return

    bad_data.selfShadowFrames.append(frame)
    with pytest.raises((vmdio_exceptions.InvalidFieldValueError, TypeError)):
        vmdio.writeVMD(bad_data, temp_vmd_path)