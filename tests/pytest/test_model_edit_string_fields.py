import pytest

import pyvmdio.model_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions
from pyvmdio.vmd_string import VMDString


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


def test_model_name_not_encodable_to_shift_jis():
    bad_data = vmdio.VMDData()

    with pytest.raises(vmdio_exceptions.StringProcessError):
        bad_data.modelName = "\N{MUSICAL KEYBOARD}"


def test_bone_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()
    frame.boneName = "B" * 16
    bad_data.motionFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_bone_name_not_encodable_to_shift_jis():
    frame = vmdio.MotionFrame()

    with pytest.raises(vmdio_exceptions.StringProcessError):
        frame.boneName = "\N{MUSICAL KEYBOARD}"


def test_morph_name_exceeds_shift_jis_limit(temp_vmd_path):
    bad_data = vmdio.VMDData()
    bad_data.modelName = "vmdio-test"

    frame = vmdio.MorphFrame()
    frame.morphName = "M" * 16
    bad_data.morphFrames.append(frame)

    with pytest.raises(vmdio_exceptions.InvalidFieldValueError):
        vmdio.writeVMD(bad_data, temp_vmd_path)


def test_morph_name_not_encodable_to_shift_jis():
    frame = vmdio.MorphFrame()

    with pytest.raises(vmdio_exceptions.StringProcessError):
        frame.morphName = "\N{MUSICAL KEYBOARD}"


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


def test_ik_bone_name_not_encodable_to_shift_jis():
    ik_data = vmdio.IKData()

    with pytest.raises(vmdio_exceptions.StringProcessError):
        ik_data.ikBoneName = "\N{MUSICAL KEYBOARD}"


def test_string_fields_accept_vmd_string():
    data = vmdio.VMDData()
    data.modelName = VMDString.fromUTF8("vmdio-test")

    frame = vmdio.MotionFrame()
    frame.boneName = VMDString.fromUTF8("センター")

    morph = vmdio.MorphFrame()
    morph.morphName = VMDString.fromUTF8("まばたき")

    ik_data = vmdio.IKData()
    ik_data.ikBoneName = VMDString.fromUTF8("左足ＩＫ")

    assert data.modelName.toUTF8() == "vmdio-test"
    assert frame.boneName.toUTF8() == "センター"
    assert morph.morphName.toUTF8() == "まばたき"
    assert ik_data.ikBoneName.toUTF8() == "左足ＩＫ"


def test_string_fields_accept_shift_jis_bytes():
    frame = vmdio.MotionFrame()

    shift_jis_bytes = "センター".encode("cp932")
    frame.boneName = shift_jis_bytes

    assert frame.boneName.toShiftJIS() == shift_jis_bytes
    assert frame.boneName.toUTF8() == "センター"


def test_invalid_shift_jis_bytes_are_preserved_and_displayed_as_question_mark(temp_vmd_path):
    data = vmdio.VMDData()
    data.modelName = "vmdio-test"

    frame = vmdio.MorphFrame()

    # 0x82 is a Shift_JIS lead byte without a trailing byte.
    frame.morphName = VMDString.fromShiftJIS(b"\x82")
    data.morphFrames.append(frame)

    assert frame.morphName.toShiftJIS() == b"\x82"
    assert frame.morphName.toUTF8ForDisplay() == "?"

    with pytest.raises(vmdio_exceptions.StringProcessError):
        frame.morphName.toUTF8()

    vmdio.writeVMD(data, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert read_back.morphFrames[0].morphName.toShiftJIS() == b"\x82"
    assert read_back.morphFrames[0].morphName.toUTF8ForDisplay() == "?"