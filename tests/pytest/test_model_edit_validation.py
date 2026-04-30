import pytest

import pyvmdio.model_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions


def test_frame_count_overflow(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    for i in range(600001):
        frame = vmdio.MotionFrame()
        frame.boneName = "ボーン1"
        frame.frameNumber = i
        bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.FrameOverflowError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_motion_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    f1 = vmdio.MotionFrame()
    f1.frameNumber = 10
    f1.boneName = "ボーン1"
    bad_data.motionFrames.append(f1)

    f2 = vmdio.MotionFrame()
    f2.frameNumber = 10
    f2.boneName = "ボーン1"
    bad_data.motionFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_morph_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    f1 = vmdio.MorphFrame()
    f1.frameNumber = 20
    f1.morphName = "シェイプキー1"
    bad_data.morphFrames.append(f1)

    f2 = vmdio.MorphFrame()
    f2.frameNumber = 20
    f2.morphName = "シェイプキー1"
    bad_data.morphFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_visible_ik_frame_conflict(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    f1 = vmdio.VisibleIKFrame()
    f1.frameNumber = 30
    bad_data.visibleIKFrames.append(f1)

    f2 = vmdio.VisibleIKFrame()
    f2.frameNumber = 30
    bad_data.visibleIKFrames.append(f2)

    with pytest.raises(vmdio_exceptions.FrameConflictError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_zero_quaternion(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()
    frame.boneName = "ボーン1"

    frame.rotation.qx = 0.0
    frame.rotation.qy = 0.0
    frame.rotation.qz = 0.0
    frame.rotation.qw = 0.0
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_invalid_motion_interpolation(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()
    frame.boneName = "ボーン1"

    frame.interpolation.xPos.x1 = 128
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)