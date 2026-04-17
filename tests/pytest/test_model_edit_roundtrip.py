import pyvmdio.model_edit as vmdio

from helpers import (
    assert_motion_frame_equal,
    assert_morph_frame_equal,
    assert_visible_ik_frame_equal,
)


def test_read_write_data_and_consistency(test_motion_data_dir, temp_vmd_path):
    src = test_motion_data_dir / "motion_morph_visibleik.vmd"

    original = vmdio.readVMD(src)
    vmdio.writeVMD(original, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert original.modelName == read_back.modelName
    assert len(original.motionFrames) == len(read_back.motionFrames)
    assert len(original.morphFrames) == len(read_back.morphFrames)
    assert len(original.visibleIKFrames) == len(read_back.visibleIKFrames)

    for f1, f2 in zip(original.motionFrames, read_back.motionFrames):
        assert_motion_frame_equal(f1, f2)

    for f1, f2 in zip(original.morphFrames, read_back.morphFrames):
        assert_morph_frame_equal(f1, f2)

    for f1, f2 in zip(original.visibleIKFrames, read_back.visibleIKFrames):
        assert_visible_ik_frame_equal(f1, f2)