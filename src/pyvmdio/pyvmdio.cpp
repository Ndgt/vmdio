#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl/filesystem.h>

#include "vmdio/camera_edit.h"
#include "vmdio/encoding.h"
#include "vmdio/exceptions.h"
#include "vmdio/model_edit.h"

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::CameraFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::LightFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::SelfShadowFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::MotionFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::MorphFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::VisibleIKFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::IKData>);

PYBIND11_MODULE(_pyvmdio, m)
{
    // Define submodules
    py::module_ mCamera = m.def_submodule("camera_edit");
    py::module_ mEncoding = m.def_submodule("encoding");
    py::module_ mExceptions = m.def_submodule("exceptions");
    py::module_ mModel = m.def_submodule("model_edit");

    // Bind types and functions in camera_edit header
    py::enum_<vmdio::camera_edit::ProjectionType>(mCamera, "ProjectionType")
        .value("Perspective", vmdio::camera_edit::ProjectionType::Perspective)
        .value("Orthographic", vmdio::camera_edit::ProjectionType::Orthographic);

    py::enum_<vmdio::camera_edit::SelfShadowMode>(mCamera, "SelfShadowMode")
        .value("NoSelfShadow", vmdio::camera_edit::SelfShadowMode::NoSelfShadow)
        .value("Mode1", vmdio::camera_edit::SelfShadowMode::Mode1)
        .value("Mode2", vmdio::camera_edit::SelfShadowMode::Mode2);

    py::class_<vmdio::camera_edit::Position>(mCamera, "Position")
        .def(py::init<>())
        .def_readwrite("x", &vmdio::camera_edit::Position::x)
        .def_readwrite("y", &vmdio::camera_edit::Position::y)
        .def_readwrite("z", &vmdio::camera_edit::Position::z);

    py::class_<vmdio::camera_edit::CameraRotation>(mCamera, "CameraRotation")
        .def(py::init<>())
        .def_readwrite("eulerX", &vmdio::camera_edit::CameraRotation::eulerX)
        .def_readwrite("eulerY", &vmdio::camera_edit::CameraRotation::eulerY)
        .def_readwrite("eulerZ", &vmdio::camera_edit::CameraRotation::eulerZ);

    py::class_<vmdio::camera_edit::ControlPointSet>(mCamera, "ControlPointSet")
        .def(py::init<>())
        .def_readwrite("x1", &vmdio::camera_edit::ControlPointSet::x1)
        .def_readwrite("y1", &vmdio::camera_edit::ControlPointSet::y1)
        .def_readwrite("x2", &vmdio::camera_edit::ControlPointSet::x2)
        .def_readwrite("y2", &vmdio::camera_edit::ControlPointSet::y2);

    py::class_<vmdio::camera_edit::CameraInterpolation>(mCamera, "CameraInterpolation")
        .def(py::init<>())
        .def_readwrite("xPos", &vmdio::camera_edit::CameraInterpolation::xPos)
        .def_readwrite("yPos", &vmdio::camera_edit::CameraInterpolation::yPos)
        .def_readwrite("zPos", &vmdio::camera_edit::CameraInterpolation::zPos)
        .def_readwrite("rot", &vmdio::camera_edit::CameraInterpolation::rot)
        .def_readwrite("dist", &vmdio::camera_edit::CameraInterpolation::dist)
        .def_readwrite("view", &vmdio::camera_edit::CameraInterpolation::view);

    py::class_<vmdio::camera_edit::Color>(mCamera, "Color")
        .def(py::init<>())
        .def_readwrite("r", &vmdio::camera_edit::Color::r)
        .def_readwrite("g", &vmdio::camera_edit::Color::g)
        .def_readwrite("b", &vmdio::camera_edit::Color::b);

    py::class_<vmdio::camera_edit::CameraFrame>(mCamera, "CameraFrame")
        .def(py::init<>())
        .def_readwrite("frameNumber", &vmdio::camera_edit::CameraFrame::frameNumber)
        .def_readwrite("distance", &vmdio::camera_edit::CameraFrame::distance)
        .def_readwrite("position", &vmdio::camera_edit::CameraFrame::position)
        .def_readwrite("rotation", &vmdio::camera_edit::CameraFrame::rotation)
        .def_readwrite("interpolation", &vmdio::camera_edit::CameraFrame::interpolation)
        .def_readwrite("viewingAngle", &vmdio::camera_edit::CameraFrame::viewingAngle)
        .def_readwrite("projectionType", &vmdio::camera_edit::CameraFrame::projectionType);

    py::class_<vmdio::camera_edit::LightFrame>(mCamera, "LightFrame")
        .def(py::init<>())
        .def_readwrite("frameNumber", &vmdio::camera_edit::LightFrame::frameNumber)
        .def_readwrite("color", &vmdio::camera_edit::LightFrame::color)
        .def_readwrite("position", &vmdio::camera_edit::LightFrame::position);

    py::class_<vmdio::camera_edit::SelfShadowFrame>(mCamera, "SelfShadowFrame")
        .def(py::init<>())
        .def_readwrite("frameNumber", &vmdio::camera_edit::SelfShadowFrame::frameNumber)
        .def_readwrite("mode", &vmdio::camera_edit::SelfShadowFrame::mode)
        .def_readwrite("shadowRange", &vmdio::camera_edit::SelfShadowFrame::shadowRange);

    py::bind_vector<std::vector<vmdio::camera_edit::CameraFrame>>(mCamera, "CameraFrameList");
    py::bind_vector<std::vector<vmdio::camera_edit::LightFrame>>(mCamera, "LightFrameList");
    py::bind_vector<std::vector<vmdio::camera_edit::SelfShadowFrame>>(
        mCamera, "SelfShadowFrameList");

    py::class_<vmdio::camera_edit::VMDData>(mCamera, "VMDData")
        .def(py::init<>())
        .def_readwrite("cameraFrames", &vmdio::camera_edit::VMDData::cameraFrames)
        .def_readwrite("lightFrames", &vmdio::camera_edit::VMDData::lightFrames)
        .def_readwrite("selfShadowFrames", &vmdio::camera_edit::VMDData::selfShadowFrames);

    mCamera.def("readVMD", &vmdio::camera_edit::readVMD,
                "Read a VMD file and return a CameraEdit.VMDData object",
                py::arg("file_path"),
                py::call_guard<py::gil_scoped_release>());

    mCamera.def("writeVMD", &vmdio::camera_edit::writeVMD,
                "Write a CameraEdit.VMDData object to a VMD file",
                py::arg("vmd_data"),
                py::arg("file_path"),
                py::call_guard<py::gil_scoped_release>());

    // Bind functions in encoding header
    mEncoding.def("utf8ToShiftJIS", &vmdio::encoding::utf8ToShiftJIS,
                  "Convert a UTF-8 string to Shift_JIS encoding",
                  py::arg("utf8_string"));

    mEncoding.def("shiftJISToUTF8", &vmdio::encoding::shiftJISToUTF8,
                  "Convert a Shift_JIS encoded string to UTF-8",
                  py::arg("shift_jis_string"));

    // Bind exceptions in exceptions header
    py::register_exception<vmdio::exceptions::VMDIOError>(mExceptions, "VMDIOError");
    py::register_exception<vmdio::exceptions::FileSystemError>(mExceptions, "FileSystemError");
    py::register_exception<vmdio::exceptions::FrameConflictError>(
        mExceptions, "FrameConflictError");
    py::register_exception<vmdio::exceptions::FrameOverflowError>(
        mExceptions, "FrameOverflowError");
    py::register_exception<vmdio::exceptions::IncompatibleFormatError>(
        mExceptions, "IncompatibleFormatError");
    py::register_exception<vmdio::exceptions::InvalidFieldValueError>(
        mExceptions, "InvalidFieldValueError");
    py::register_exception<vmdio::exceptions::StringProcessError>(
        mExceptions, "StringProcessError");

    // Bind types and functions in model_edit header
    py::enum_<vmdio::model_edit::IKState>(mModel, "IKState")
        .value("OFF", vmdio::model_edit::IKState::OFF)
        .value("ON", vmdio::model_edit::IKState::ON);

    py::enum_<vmdio::model_edit::Visibility>(mModel, "Visibility")
        .value("Hidden", vmdio::model_edit::Visibility::Hidden)
        .value("Visible", vmdio::model_edit::Visibility::Visible);

    py::class_<vmdio::model_edit::Position>(mModel, "Position")
        .def(py::init<>())
        .def_readwrite("x", &vmdio::model_edit::Position::x)
        .def_readwrite("y", &vmdio::model_edit::Position::y)
        .def_readwrite("z", &vmdio::model_edit::Position::z);

    py::class_<vmdio::model_edit::Quaternion>(mModel, "Quaternion")
        .def(py::init<>())
        .def_readwrite("qx", &vmdio::model_edit::Quaternion::qx)
        .def_readwrite("qy", &vmdio::model_edit::Quaternion::qy)
        .def_readwrite("qz", &vmdio::model_edit::Quaternion::qz)
        .def_readwrite("qw", &vmdio::model_edit::Quaternion::qw);

    py::class_<vmdio::model_edit::ControlPointSet>(mModel, "ControlPointSet")
        .def(py::init<>())
        .def_readwrite("x1", &vmdio::model_edit::ControlPointSet::x1)
        .def_readwrite("y1", &vmdio::model_edit::ControlPointSet::y1)
        .def_readwrite("x2", &vmdio::model_edit::ControlPointSet::x2)
        .def_readwrite("y2", &vmdio::model_edit::ControlPointSet::y2);

    py::class_<vmdio::model_edit::MotionInterpolation>(mModel, "MotionInterpolation")
        .def(py::init<>())
        .def_readwrite("xPos", &vmdio::model_edit::MotionInterpolation::xPos)
        .def_readwrite("yPos", &vmdio::model_edit::MotionInterpolation::yPos)
        .def_readwrite("zPos", &vmdio::model_edit::MotionInterpolation::zPos)
        .def_readwrite("rot", &vmdio::model_edit::MotionInterpolation::rot);

    py::class_<vmdio::model_edit::MotionFrame>(mModel, "MotionFrame")
        .def(py::init<>())
        .def_readwrite("boneName", &vmdio::model_edit::MotionFrame::boneName)
        .def_readwrite("frameNumber", &vmdio::model_edit::MotionFrame::frameNumber)
        .def_readwrite("position", &vmdio::model_edit::MotionFrame::position)
        .def_readwrite("rotation", &vmdio::model_edit::MotionFrame::rotation)
        .def_readwrite("interpolation", &vmdio::model_edit::MotionFrame::interpolation);

    py::class_<vmdio::model_edit::MorphFrame>(mModel, "MorphFrame")
        .def(py::init<>())
        .def_readwrite("morphName", &vmdio::model_edit::MorphFrame::morphName)
        .def_readwrite("frameNumber", &vmdio::model_edit::MorphFrame::frameNumber)
        .def_readwrite("value", &vmdio::model_edit::MorphFrame::value);

    py::class_<vmdio::model_edit::IKData>(mModel, "IKData")
        .def(py::init<>())
        .def_readwrite("ikBoneName", &vmdio::model_edit::IKData::ikBoneName)
        .def_readwrite("ikState", &vmdio::model_edit::IKData::ikState);

    py::bind_vector<std::vector<vmdio::model_edit::IKData>>(mModel, "IKDataList");

    py::class_<vmdio::model_edit::VisibleIKFrame>(mModel, "VisibleIKFrame")
        .def(py::init<>())
        .def_readwrite("frameNumber", &vmdio::model_edit::VisibleIKFrame::frameNumber)
        .def_readwrite("visibility", &vmdio::model_edit::VisibleIKFrame::visibility)
        .def_readwrite("ikDataList", &vmdio::model_edit::VisibleIKFrame::ikDataList);

    py::bind_vector<std::vector<vmdio::model_edit::MotionFrame>>(mModel, "MotionFrameList");
    py::bind_vector<std::vector<vmdio::model_edit::MorphFrame>>(mModel, "MorphFrameList");
    py::bind_vector<std::vector<vmdio::model_edit::VisibleIKFrame>>(mModel, "VisibleIKFrameList");

    py::class_<vmdio::model_edit::VMDData>(mModel, "VMDData")
        .def(py::init<>())
        .def_readwrite("modelName", &vmdio::model_edit::VMDData::modelName)
        .def_readwrite("motionFrames", &vmdio::model_edit::VMDData::motionFrames)
        .def_readwrite("morphFrames", &vmdio::model_edit::VMDData::morphFrames)
        .def_readwrite("visibleIKFrames", &vmdio::model_edit::VMDData::visibleIKFrames);

    mModel.def("readVMD", &vmdio::model_edit::readVMD,
               "Read a VMD file and return a ModelEdit.VMDData object",
               py::arg("file_path"),
               py::call_guard<py::gil_scoped_release>());

    mModel.def("writeVMD", &vmdio::model_edit::writeVMD,
               "Write a ModelEdit.VMDData object to a VMD file",
               py::arg("vmd_data"),
               py::arg("file_path"),
               py::call_guard<py::gil_scoped_release>());
}