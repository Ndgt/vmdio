import math

import pyvmdio.model_edit as vmdio


def test_non_unit_quaternion_is_normalized_on_write(temp_vmd_path):
    data = vmdio.VMDData()
    data.modelName = "vmdio-test"

    frame = vmdio.MotionFrame()
    frame.boneName = "ボーン1"

    frame.rotation.qx = 0.0
    frame.rotation.qy = 0.0
    frame.rotation.qz = 0.0
    frame.rotation.qw = 2.0
    data.motionFrames.append(frame)

    vmdio.writeVMD(data, temp_vmd_path)
    read_back = vmdio.readVMD(temp_vmd_path)

    assert math.isclose(frame.rotation.qx, 0.0, abs_tol=1e-5)
    assert math.isclose(frame.rotation.qy, 0.0, abs_tol=1e-5)
    assert math.isclose(frame.rotation.qz, 0.0, abs_tol=1e-5)
    assert math.isclose(frame.rotation.qw, 2.0, abs_tol=1e-5)

    assert math.isclose(read_back.motionFrames[0].rotation.qx, 0.0, abs_tol=1e-5)
    assert math.isclose(read_back.motionFrames[0].rotation.qy, 0.0, abs_tol=1e-5)
    assert math.isclose(read_back.motionFrames[0].rotation.qz, 0.0, abs_tol=1e-5)
    assert math.isclose(read_back.motionFrames[0].rotation.qw, 1.0, abs_tol=1e-5)