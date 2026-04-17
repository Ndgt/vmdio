import pyvmdio.camera_edit as vmdio

from helpers import (
    assert_camera_frame_equal,
    assert_light_frame_equal,
    assert_self_shadow_frame_equal,
)


def test_read_write_camera_data_and_consistency(test_motion_data_dir, temp_vmd_path):
    src = test_motion_data_dir / "camera.vmd"
    original = vmdio.readVMD(src)
    vmdio.writeVMD(original, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert len(original.cameraFrames) == len(read_back.cameraFrames)
    for f1, f2 in zip(original.cameraFrames, read_back.cameraFrames):
        assert_camera_frame_equal(f1, f2)


def test_read_write_light_data_and_consistency(test_motion_data_dir, temp_vmd_path):
    src = test_motion_data_dir / "light.vmd"
    original = vmdio.readVMD(src)
    vmdio.writeVMD(original, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert len(original.lightFrames) == len(read_back.lightFrames)
    for f1, f2 in zip(original.lightFrames, read_back.lightFrames):
        assert_light_frame_equal(f1, f2)


def test_read_write_self_shadow_data_and_consistency(test_motion_data_dir, temp_vmd_path):
    src = test_motion_data_dir / "selfshadow.vmd"
    original = vmdio.readVMD(src)
    vmdio.writeVMD(original, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert len(original.selfShadowFrames) == len(read_back.selfShadowFrames)
    for f1, f2 in zip(original.selfShadowFrames, read_back.selfShadowFrames):
        assert_self_shadow_frame_equal(f1, f2)