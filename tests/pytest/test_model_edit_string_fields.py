import pytest

import pyvmdio.model_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions


def test_empty_model_name(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = ""

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)

def test_empty_bone_name(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()

    frame.boneName = ""
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_empty_morph_name(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MorphFrame()

    frame.morphName = ""
    bad_data.morphFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_empty_ik_bone_name(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.VisibleIKFrame()


    ik_data = vmdio.IKData()
    ik_data.ikBoneName = ""
    frame.ikDataList.append(ik_data)

    bad_data.visibleIKFrames.append(frame)
    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_model_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "A" * 21

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)

def test_model_name_not_encodable_to_shift_jis(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "\N{MUSICAL KEYBOARD}"

    with pytest.raises(vmdio_exceptions.StringProcessError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_bone_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()

    frame.boneName = "B" * 16
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_bone_name_not_encodable_to_shift_jis(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()

    frame.boneName = "\N{MUSICAL KEYBOARD}"
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.StringProcessError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_morph_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MorphFrame()

    frame.morphName = "M" * 16
    bad_data.morphFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_morph_name_not_encodable_to_shift_jis(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MorphFrame()

    frame.morphName = "\N{MUSICAL KEYBOARD}"
    bad_data.morphFrames.append(frame)

    with pytest.raises(vmdio_exceptions.StringProcessError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_ik_bone_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.VisibleIKFrame()


    ik_data = vmdio.IKData()
    ik_data.ikBoneName = "I" * 21
    frame.ikDataList.append(ik_data)

    bad_data.visibleIKFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_ik_bone_name_not_encodable_to_shift_jis(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.VisibleIKFrame()


    ik_data = vmdio.IKData()
    ik_data.ikBoneName = "\N{MUSICAL KEYBOARD}"
    frame.ikDataList.append(ik_data)

    bad_data.visibleIKFrames.append(frame)

    with pytest.raises(vmdio_exceptions.StringProcessError):
        vmdio.writeVMD(bad_data, temp_vmd_path)