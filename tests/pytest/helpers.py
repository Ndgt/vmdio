import math

FLOAT_TOL = 1e-5


def assert_vmd_string_equal(s1, s2):
    assert s1.toShiftJIS() == s2.toShiftJIS()


def assert_control_point_set_equal(cp1, cp2):
    assert cp1.x1 == cp2.x1
    assert cp1.y1 == cp2.y1
    assert cp1.x2 == cp2.x2
    assert cp1.y2 == cp2.y2


def assert_motion_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert_vmd_string_equal(f1.boneName, f2.boneName)

    assert math.isclose(f1.position.x, f2.position.x, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.y, f2.position.y, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.z, f2.position.z, abs_tol=FLOAT_TOL)

    assert math.isclose(f1.rotation.qx, f2.rotation.qx, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.rotation.qy, f2.rotation.qy, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.rotation.qz, f2.rotation.qz, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.rotation.qw, f2.rotation.qw, abs_tol=FLOAT_TOL)

    assert_control_point_set_equal(f1.interpolation.xPos, f2.interpolation.xPos)
    assert_control_point_set_equal(f1.interpolation.yPos, f2.interpolation.yPos)
    assert_control_point_set_equal(f1.interpolation.zPos, f2.interpolation.zPos)
    assert_control_point_set_equal(f1.interpolation.rot, f2.interpolation.rot)


def assert_morph_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert_vmd_string_equal(f1.morphName, f2.morphName)
    assert math.isclose(f1.value, f2.value, abs_tol=FLOAT_TOL)


def assert_visible_ik_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert f1.visibility == f2.visibility
    assert len(f1.ikDataList) == len(f2.ikDataList)

    for ik1, ik2 in zip(f1.ikDataList, f2.ikDataList):
        assert_vmd_string_equal(ik1.ikBoneName, ik2.ikBoneName)
        assert ik1.ikState == ik2.ikState


def assert_camera_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert math.isclose(f1.distance, f2.distance, abs_tol=FLOAT_TOL)

    assert math.isclose(f1.position.x, f2.position.x, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.y, f2.position.y, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.z, f2.position.z, abs_tol=FLOAT_TOL)

    assert math.isclose(f1.rotation.eulerX, f2.rotation.eulerX, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.rotation.eulerY, f2.rotation.eulerY, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.rotation.eulerZ, f2.rotation.eulerZ, abs_tol=FLOAT_TOL)

    assert_control_point_set_equal(f1.interpolation.xPos, f2.interpolation.xPos)
    assert_control_point_set_equal(f1.interpolation.yPos, f2.interpolation.yPos)
    assert_control_point_set_equal(f1.interpolation.zPos, f2.interpolation.zPos)
    assert_control_point_set_equal(f1.interpolation.rot, f2.interpolation.rot)
    assert_control_point_set_equal(f1.interpolation.dist, f2.interpolation.dist)
    assert_control_point_set_equal(f1.interpolation.view, f2.interpolation.view)

    assert f1.viewingAngle == f2.viewingAngle
    assert f1.projectionType == f2.projectionType


def assert_light_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert f1.color.r == f2.color.r
    assert f1.color.g == f2.color.g
    assert f1.color.b == f2.color.b

    assert math.isclose(f1.position.x, f2.position.x, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.y, f2.position.y, abs_tol=FLOAT_TOL)
    assert math.isclose(f1.position.z, f2.position.z, abs_tol=FLOAT_TOL)


def assert_self_shadow_frame_equal(f1, f2):
    assert f1.frameNumber == f2.frameNumber
    assert f1.mode == f2.mode
    assert f1.shadowRange == f2.shadowRange